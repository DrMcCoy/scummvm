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

#ifndef DARKSEED2_MOVIE_H
#define DARKSEED2_MOVIE_H

#include "common/rect.h"

#include "audio/mixer.h"

#include "darkseed2/darkseed2.h"
#include "darkseed2/sprite.h"
#include "darkseed2/saveable.h"

namespace Common {
	class String;
}

namespace Video {
	class VideoDecoder;
}

namespace DarkSeed2 {

class Graphics;
class Cursors;
class Sound;

class Movie : public Saveable {
public:
	Movie(Audio::Mixer &mixer, Graphics &graphics, Cursors &cursors, Sound &sound, Common::Platform platform);
	~Movie();

	/** Is a movie currently playing? */
	bool isPlaying() const;

	/** Play that movie. */
	bool play(const Common::String &file, int32 x = 0, int32 y = 0);

	/** Stop playing that movie. */
	void stop();

	/** Check for status changes. */
	void updateStatus();

	/** Redraw the movie frame. */
	void redraw(Sprite &sprite, Common::Rect area);

	/** Return the time to wait until the next frame can be displayed. */
	uint32 getFrameWaitTime() const;

protected:
	bool saveLoad(Common::Serializer &serializer, Resources &resources);
	bool loading(Resources &resources);

private:
	static const bool _doubleHalfSizedVideos = true;

	Audio::Mixer *_mixer;
	Graphics     *_graphics;
	Cursors      *_cursors;
	Sound        *_sound;

	Common::String _fileName; ///< The current video's file name.

	Common::Platform _platform;

	int32 _x; ///< The X origin.
	int32 _y; ///< The Y origin.

	Common::Rect _area; ///< The movie's area.

	bool _doubling;      ///< Double the video's resolution?
	bool _cursorVisible; ///< Was the cursor visible at the start?

	/** The video decoder. */
	Video::VideoDecoder *_decoder;

	Sprite _screen; ///< The current frame's sprite.

	Video::VideoDecoder *createDecoder(const Common::String &file) const;
};

} // End of namespace DarkSeed2

#endif // DARKSEED2_MOVIE_H
