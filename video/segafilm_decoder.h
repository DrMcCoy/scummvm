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
 */

#ifndef VIDEO_SEGAFILM_DECODER_H
#define VIDEO_SEGAFILM_DECODER_H

#include "video/video_decoder.h"

namespace Audio {
class QueuingAudioStream;
}

namespace Common {
class SeekableReadStream;
}

namespace Image {
class Codec;
}

namespace Video {

class SegaFILMDecoder : public VideoDecoder {
public:
	SegaFILMDecoder();
	~SegaFILMDecoder();

	bool loadStream(Common::SeekableReadStream *stream);
	void close();

protected:
	void readNextPacket();

private:
	class SegaFILMVideoTrack : public VideoTrack {
	public:
		SegaFILMVideoTrack(uint32 width, uint32 height, uint32 codecTag, byte bitsPerPixel, uint32 frameCount, uint32 timeScale);
		~SegaFILMVideoTrack();

		uint16 getWidth() const { return _width; }
		uint16 getHeight() const { return _height; }
		Graphics::PixelFormat getPixelFormat() const;
		int getCurFrame() const { return _curFrame; }
		int getFrameCount() const { return _frameCount; }
		uint32 getNextFrameStartTime() const { return _nextFrameStartTime * 1000 / _timeScale; }
		const Graphics::Surface *decodeNextFrame() { return _surface; }

		void decodeFrame(Common::SeekableReadStream *stream, uint32 duration);

	private:
		const Graphics::Surface *_surface;
		uint32 _timeScale;
		uint32 _nextFrameStartTime;

		uint32 _frameCount;
		Image::Codec *_codec;
		uint16 _width, _height;
		int _curFrame;
	};

	class SegaFILMAudioTrack : public AudioTrack {
	public:
		SegaFILMAudioTrack(uint audioFrequency, uint audioFlags);
		~SegaFILMAudioTrack();

		void queueAudio(Common::SeekableReadStream *stream, uint32 length);

	protected:
		Audio::AudioStream *getAudioStream() const;

	private:
		Audio::QueuingAudioStream *_audioStream;
		uint16 _audioFlags;
	};

	Common::SeekableReadStream *_stream;

	struct SampleTableEntry {
		uint32 offset;
		uint32 length;
		uint32 sampleInfo1;
		uint32 sampleInfo2;
	};

	uint32 _sampleTablePosition;
	SampleTableEntry *_sampleTable;
};

} // End of namespace Video

#endif
