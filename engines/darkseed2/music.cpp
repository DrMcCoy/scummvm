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

Music::Music(Audio::Mixer &mixer, MidiDriver &driver) : _mixer(&mixer), _midiDriver(&driver) {
	_midiParser = 0;
	_midiDriver->open();

	_midiMode = kMidiModeGM;
	_midiData = 0;

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

void Music::init(MusicType musicType) {
	_musicType = musicType;

	switch (_musicType) {
	case kMusicTypeSMF:
		_midiParser = MidiParser::createParser_SMF();
		break;
	case kMusicTypeSEQ:
		warning("Unhandled Saturn SEQ files");
		break;
	case kMusicTypeQT:
		_midiParser = MidiParser::createParser_QT();
		break;
	}

	if (_midiParser) {
		_midiParser->setMidiDriver(_midiDriver);
		_midiParser->setTimerRate(_midiDriver->getBaseTempo());
		_midiParser->property(MidiParser::mpAutoLoop, 1);
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

	_midiData = new byte[mid.size()];
	mid.read(_midiData, mid.size());

	_midiParser->loadMusic(_midiData, mid.size());
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

	if (_musicType == kMusicTypeSMF) {
		if      (_midiMode == kMidiModeGM)
			midFile += "gm";
		else if (_midiMode == kMidiModeFM)
			midFile += "fm";
	}

	midFile = Resources::addExtension(midFile,
			resources.getVersionFormats().getMusicExtension(_musicType));

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
	byte midiMode = (byte) _midiMode;

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

} // End of namespace DarkSeed2
