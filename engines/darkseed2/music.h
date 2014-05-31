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

#ifndef DARKSEED2_MUSIC_H
#define DARKSEED2_MUSIC_H

#include "common/mutex.h"

#include "audio/mixer.h"
#include "audio/mididrv.h"

#include "darkseed2/darkseed2.h"
#include "darkseed2/saveable.h"

namespace Common {
	class SeekableReadStream;
	class String;
}

class MidiParser;

namespace DarkSeed2 {

class Resources;

class Resource;
class MidiPlayer;

class Music : public Saveable {
public:
	/** The MIDI mode. */
	enum MidiMode {
		kMidiModeGM = 0, ///< General MIDI
		kMidiModeFM = 1  ///< Frequency modulation synthesis
	};

	Music(Common::Platform platform, Audio::Mixer &mixer, MidiDriver &driver);
	~Music();

	/** Set the MIDI mode. */
	void setMidiMode(MidiMode midiMode);

	/** Play that MIDI file. */
	bool playMID(Resources &resources, const Common::String &mid);

	/** Stop the music. */
	void stop();

protected:
	bool saveLoad(Common::Serializer &serializer, Resources &resources);
	bool loading(Resources &resources);

private:
	Common::Platform _platform;

	Audio::Mixer *_mixer;

	MidiDriver *_midiDriver;
	MidiParser *_midiParser;
	byte *_midiData;

	Common::String _name; ///< The currently playing music.
	MidiMode _midiMode;   ///< The current MIDI mode.

	MusicType _musicType;

	bool _mute; ///< Muted?

	bool playMID(Common::SeekableReadStream &mid);
	bool playMID(const Resource &resource);

	byte *convertSEQ(Common::SeekableReadStream &mid, uint32 &size);
};

} // End of namespace DarkSeed2

#endif // DARKSEED2_MUSIC_H
