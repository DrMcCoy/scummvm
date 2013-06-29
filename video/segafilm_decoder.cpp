/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/endian.h"
#include "common/scummsys.h"
#include "common/stream.h"
#include "common/system.h"
#include "common/textconsole.h"

#include "audio/audiostream.h"
#include "audio/decoders/raw.h"

#include "video/segafilm_decoder.h"
#include "video/codecs/cinepak.h"

namespace Video {

// TODO: Think about moving this to a separate codecs/raw.h file
// For raw video, it seems to always be 24bpp RGB
// We just convert to the current screen format for ease of use
class SegaFilmRawCodec : public Video::Codec {
public:
	SegaFilmRawCodec(uint16 width, uint16 height, byte bitsPerPixel) {
		_surface = new Graphics::Surface();
		_surface->create(width, height, g_system->getScreenFormat());
		_bitsPerPixel = bitsPerPixel;
	}

	~SegaFilmRawCodec() {
		_surface->free();
		delete _surface;
	}

	const Graphics::Surface *decodeImage(Common::SeekableReadStream *stream) {
		if (_bitsPerPixel != 24) {
			warning("Unhandled %d bpp", _bitsPerPixel);
			return 0;
		}

		if (stream->size() != _surface->w * _surface->h * (_bitsPerPixel >> 3)) {
			warning("Mismatched raw video size");
			return 0;
		}

		for (int32 i = 0; i < _surface->w * _surface->h; i++) {
			byte r = stream->readByte();
			byte g = stream->readByte();
			byte b = stream->readByte();

			if (_surface->format.bytesPerPixel == 2)
				*((uint16 *)_surface->pixels + i) = _surface->format.RGBToColor(r, g, b);
			else
				*((uint32 *)_surface->pixels + i) = _surface->format.RGBToColor(r, g, b);
		}

		return _surface;
	}

	Graphics::PixelFormat getPixelFormat() const { return _surface->format; }

private:
	Graphics::Surface *_surface;
	byte _bitsPerPixel;
};

SegaFILMDecoder::SegaFILMDecoder() : _stream(0), _sampleTable(0) {
}

SegaFILMDecoder::~SegaFILMDecoder() {
	close();
}

bool SegaFILMDecoder::loadStream(Common::SeekableReadStream *stream) {
	close();

	_stream = stream;

	// FILM Header
	if (_stream->readUint32BE() != MKTAG('F', 'I', 'L', 'M')) {
		close();
		return false;
	}

	uint32 filmHeaderLength = _stream->readUint32BE();
	uint32 filmVersion = _stream->readUint32BE();
	_stream->readUint32BE(); // Reserved

	// We don't support the 3DO/SegaCD/Batman and Robin variants
	if (filmVersion == 0 || filmVersion == 0x20000) {
		close();
		return false;
	}

	// FDSC Chunk
	if (_stream->readUint32BE() != MKTAG('F', 'D', 'S', 'C')) {
		close();
		return false;
	}

	/* uint32 fdscChunkSize = */ _stream->readUint32BE();
	uint32 codecTag = _stream->readUint32BE();
	uint32 height = _stream->readUint32BE();
	uint32 width = _stream->readUint32BE();
	byte bitsPerPixel = _stream->readByte();
	byte audioChannels = _stream->readByte();
	byte audioSampleSize = _stream->readByte();
	_stream->readByte(); // Unknown
	uint16 audioFrequency = _stream->readUint16BE();

	if (codecTag == 0) {
		warning("Unsupported audio-only Sega FILM file");
		close();
		return false;
	}

	_stream->skip(6);

	// STAB Chunk
	if (_stream->readUint32BE() != MKTAG('S', 'T', 'A', 'B')) {
		close();
		return false;
	}

	// The STAB chunk size changes definitions depending on the version anyway...
	/* uint32 stabChunkSize = */ _stream->readUint32BE();
	uint32 timeScale = _stream->readUint32BE();
	uint32 sampleCount = _stream->readUint32BE();

	_sampleTable = new SampleTableEntry[sampleCount];
	uint32 frameCount = 0;

	for (uint32 i = 0; i < sampleCount; i++) {
		_sampleTable[i].offset = _stream->readUint32BE() + filmHeaderLength; // Offset is relative to the end of the header
		_sampleTable[i].length = _stream->readUint32BE();
		_sampleTable[i].sampleInfo1 = _stream->readUint32BE();
		_sampleTable[i].sampleInfo2 = _stream->readUint32BE();

		// Calculate the frame count based on the number of video samples.
		// 0xFFFFFFFF represents an audio frame.
		if (_sampleTable[i].sampleInfo1 != 0xFFFFFFFF)
			frameCount++;
	}

	addTrack(new SegaFILMVideoTrack(width, height, codecTag, bitsPerPixel, frameCount, timeScale));

	// Create the audio stream if audio is present
	if (audioSampleSize != 0) {
		uint audioFlags = 0;
		if (audioChannels == 2)
			audioFlags |= Audio::FLAG_STEREO;
		if (audioSampleSize == 16)
			audioFlags |= Audio::FLAG_16BITS;

		addTrack(new SegaFILMAudioTrack(audioFrequency, audioFlags));
	}

	_sampleTablePosition = 0;

	return true;
}

void SegaFILMDecoder::close() {
	VideoDecoder::close();

	delete[] _sampleTable; _sampleTable = 0;
	delete _stream; _stream = 0;
}

void SegaFILMDecoder::readNextPacket() {
	if (endOfVideoTracks())
		return;

	// Decode until we get a video frame
	for (;;) {
		_stream->seek(_sampleTable[_sampleTablePosition].offset);

		if (_sampleTable[_sampleTablePosition].sampleInfo1 == 0xFFFFFFFF) {
			// Planar audio data. All left channel first and then left in stereo.
			SegaFILMAudioTrack *audioTrack = (SegaFILMAudioTrack *)getTrack(1);
			assert(audioTrack);
			audioTrack->queueAudio(_stream, _sampleTable[_sampleTablePosition].length);
			_sampleTablePosition++;
		} else {
			// We have a video frame!
			((SegaFILMVideoTrack *)getTrack(0))->decodeFrame(_stream->readStream(_sampleTable[_sampleTablePosition].length), _sampleTable[_sampleTablePosition].sampleInfo2);
			_sampleTablePosition++;
			return;
		}
	}

	// This should be impossible to get to
	error("Could not find Sega FILM frame %d", getCurFrame());
}

SegaFILMDecoder::SegaFILMVideoTrack::SegaFILMVideoTrack(uint32 width, uint32 height, uint32 codecTag, byte bitsPerPixel, uint32 frameCount, uint32 timeScale) {
	_width = width;
	_height = height;
	_frameCount = frameCount;
	_timeScale = timeScale;
	_curFrame = -1;
	_nextFrameStartTime = 0;

	// Create the Cinepak decoder, if we're using it
	if (codecTag == MKTAG('c', 'v', 'i', 'd'))
		_codec = new Video::CinepakDecoder();
	else if (codecTag == MKTAG('r', 'a', 'w', ' '))
		_codec = new SegaFilmRawCodec(_width, _height, bitsPerPixel);
	else if (codecTag != 0)
		error("Unknown Sega FILM codec tag '%s'", tag2str(codecTag));
}

SegaFILMDecoder::SegaFILMVideoTrack::~SegaFILMVideoTrack() {
	delete _codec;
}

Graphics::PixelFormat SegaFILMDecoder::SegaFILMVideoTrack::getPixelFormat() const {
	return _codec->getPixelFormat();
}

void SegaFILMDecoder::SegaFILMVideoTrack::decodeFrame(Common::SeekableReadStream *stream, uint32 duration) {
	_surface = _codec->decodeImage(stream);
	_curFrame++;
	_nextFrameStartTime += duration; // Add the frame's duration to the next frame start
}

SegaFILMDecoder::SegaFILMAudioTrack::SegaFILMAudioTrack(uint audioFrequency, uint audioFlags) {
	_audioFlags = audioFlags;
	_audioStream = Audio::makeQueuingAudioStream(audioFrequency, audioFlags & Audio::FLAG_STEREO);
}

SegaFILMDecoder::SegaFILMAudioTrack::~SegaFILMAudioTrack() {
	delete _audioStream;
}

void SegaFILMDecoder::SegaFILMAudioTrack::queueAudio(Common::SeekableReadStream *stream, uint32 length) {
	// TODO: Maybe move this to a new class?
	// TODO: CRI ADX ADPCM is possible here too
	byte *audioBuffer = (byte *)malloc(length);

	if (_audioFlags & Audio::FLAG_16BITS) {
		if (_audioFlags & Audio::FLAG_STEREO) {
			for (byte i = 0; i < 2; i++)
				for (uint16 j = 0; j < length / 4; j++)
					WRITE_BE_UINT16(audioBuffer + j * 4 + i * 2, stream->readUint16BE());
		} else {
			for (uint16 i = 0; i < length / 2; i++)
				WRITE_BE_UINT16(audioBuffer + i * 2, stream->readUint16BE());
		}
	} else {
		if (_audioFlags & Audio::FLAG_STEREO) {
			for (byte i = 0; i < 2; i++)
				for (uint16 j = 0; j < length / 2; j++)
					audioBuffer[j * 2 + i] = stream->readByte();
		} else {
			stream->read(audioBuffer, length);
		}
	}

	// Now the audio is loaded, so let's queue it
	_audioStream->queueBuffer(audioBuffer, length, DisposeAfterUse::YES, _audioFlags);
}

Audio::AudioStream *SegaFILMDecoder::SegaFILMAudioTrack::getAudioStream() const {
	return _audioStream;
}

} // End of namespace Video
