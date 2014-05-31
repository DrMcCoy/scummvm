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

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/file.h"
#include "common/serializer.h"

#include "audio/midiparser.h"

#include "darkseed2/music.h"
#include "darkseed2/resources.h"
#include "darkseed2/saveload.h"

namespace DarkSeed2 {

Music::Music(Common::Platform platform, Audio::Mixer &mixer, MidiDriver &driver) : _platform(platform), _mixer(&mixer), _midiDriver(&driver) {
	_midiDriver->open();
	_midiMode = kMidiModeGM;
	_midiData = 0;
	_midiParser = 0;

	switch (_platform) {
	case Common::kPlatformPSX:
	case Common::kPlatformWindows:
		_midiParser = MidiParser::createParser_SMF();
		break;
	case Common::kPlatformSaturn:
		warning("Unhandled Saturn SEQ files");
		break;
	case Common::kPlatformMacintosh:
		_midiParser = MidiParser::createParser_QT();
		break;
	default:
		warning("Unknown MIDI files");
		break;
	}

	if (_midiParser) {
		_midiParser->setMidiDriver(_midiDriver);
		_midiParser->setTimerRate(_midiDriver->getBaseTempo());
		_midiParser->property(MidiParser::mpAutoLoop, 1);
	}

	// TODO: Load cmf.ins with the instrument table.  It seems that an
	// interface for such an operation is supported for Adlib.  Maybe for
	// this card, setting instruments is necessary.
}

Music::~Music() {
	stop();

	_midiDriver->close();

	if (_midiParser) {
		_midiParser->setMidiDriver(0);
		delete _midiParser;
	}
}

void Music::setMidiMode(MidiMode midiMode) {
	_midiMode = midiMode;
}

bool Music::playMID(Common::SeekableReadStream &mid) {
	if (!_midiParser) {
		// Unsupported MIDI format
		return false;
	}

	uint32 size = 0;

	if (_platform == Common::kPlatformPSX) {
		_midiData = convertSEQ(mid, size);
	} else {
		size = mid.size();
		_midiData = new byte[size];
		mid.read(_midiData, size);
	}

	_midiParser->loadMusic(_midiData, size);
	_midiDriver->setTimerCallback(_midiParser, MidiParser::timerCallback);
	return true;
}

bool Music::playMID(Resources &resources, const Common::String &mid) {
	if (!_midiParser) {
		// Unsupported MIDI format
		return false;
	}

	if (mid.empty()) {
		// No file specified, stop playback

		stop();
		return true;
	}

	if (mid == _name)
		// If the current music is already playing, don't restart it
		return true;

	debugC(-1, kDebugMusic, "Playing MIDI \"%s\"", mid.c_str());

	Common::String midFile = mid;

	switch (_platform) {
	case Common::kPlatformWindows:
		if (_midiMode == kMidiModeGM)
			midFile += "gm";
		else if (_midiMode == kMidiModeFM)
			midFile += "fm";

		midFile = Resources::addExtension(midFile, "MID");
		break;
	case Common::kPlatformPSX:
	case Common::kPlatformSaturn:
		midFile = Resources::addExtension(midFile, "SEQ");
		break;
	default:
		break;
	}

	if (!resources.hasResource(midFile))
		return false;

	Common::SeekableReadStream *resMID = resources.getResource(midFile);

	bool result = playMID(*resMID);

	delete resMID;

	_name = mid;

	return result;
}

void Music::stop() {
	debugC(-1, kDebugMusic, "Stopping music");

	_name.clear();

	if (_midiParser)
		_midiParser->unloadMusic();

	delete[] _midiData; _midiData = 0;
}

bool Music::saveLoad(Common::Serializer &serializer, Resources &resources) {
	byte midiMode = (byte)_midiMode;

	SaveLoad::sync(serializer, midiMode);
	SaveLoad::sync(serializer, _name);

	_midiMode = (MidiMode) midiMode;

	return true;
}

bool Music::loading(Resources &resources) {
	Common::String name = _name;

	stop();
	playMID(resources, name);

	return true;
}

byte *Music::convertSEQ(Common::SeekableReadStream &mid, uint32 &size) {
	// Convert from PSX SEQ to normal SMF

	// Check the header
	if (mid.readUint32LE() != MKTAG('S', 'E', 'Q', 'p'))
		error("Failed to find SEQp tag");

	// Make sure we don't have a SEP file (with multiple SEQ's inside)
	if (mid.readUint32BE() != 1)
		error("Can only play SEQ files, not SEP");

	uint16 ppqn = mid.readUint16BE();
	uint32 tempo = mid.readUint16BE() << 8;
	tempo |= mid.readByte();
	/* uint16 beat = */ mid.readUint16BE();

	// Calculate the SMF size
	uint32 bodySize = mid.size() - 15;
	size = bodySize + 7 + 22; // Body + fake tempo change + header

	byte *buffer = new byte[size];

	// Construct the header
	WRITE_BE_UINT32(buffer, MKTAG('M', 'T', 'h', 'd'));
	WRITE_BE_UINT32(buffer + 4, 6); // header size
	WRITE_BE_UINT16(buffer + 8, 0); // type 0
	WRITE_BE_UINT16(buffer + 10, 1); // one track
	WRITE_BE_UINT16(buffer + 12, ppqn);
	WRITE_BE_UINT32(buffer + 14, MKTAG('M', 'T', 'r', 'k'));
	WRITE_BE_UINT32(buffer + 18, bodySize + 7); // SEQ data size + tempo change event size

	// Add a fake tempo change event
	WRITE_BE_UINT32(buffer + 22, 0x00FF5103); // no delta, meta event, tempo change, param size = 3
	WRITE_BE_UINT16(buffer + 26, tempo >> 8);
	buffer[28] = tempo & 0xFF;

	// Read in the rest of the body
	mid.read(buffer + 29, bodySize);

	return buffer;
}

} // End of namespace DarkSeed2
