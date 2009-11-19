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

#include "common/stream.h"

#include "engines/darkseed2/graphics.h"
#include "engines/darkseed2/resources.h"

namespace DarkSeed2 {

Graphics::Graphics() {
	clearPalette();

	_screen.create(_screenWidth, _screenHeight);

	_dirtyAll = false;

	_background = 0;
	_talkLine.create(_screenWidth, _screenHeight);

	_hasTalk = false;
}

Graphics::~Graphics() {
}

void Graphics::clearPalette() {
	memset(_gamePalette, 0, 768);
	applyGamePalette();
}

void Graphics::setPalette(const byte *pal) {
	memcpy(_gamePalette, pal, 768);

	// Palette entry 0 is transparent
	_gamePalette[0] = 0;
	_gamePalette[1] = 0;
	_gamePalette[2] = 0;

	applyGamePalette();
}

void Graphics::applyGamePalette() {
	byte pal[1024];

	const byte *gPal = _gamePalette;
	byte *sPal = pal;

	for (int i = 0 ; i < 256; i++, gPal += 3, sPal += 4) {
		sPal[0] = gPal[0];
		sPal[1] = gPal[1];
		sPal[2] = gPal[2];
		sPal[3] = 255;
	}

	g_system->setPalette(pal, 0, 256);
}

void Graphics::drawString(const Common::String &string, int x, int y, byte color) {
	Common::Rect coords;

	_screen.drawString(string, x, y, color, &coords);

	dirtyRectsAdd(coords);
}

void Graphics::talk(const Common::String &string) {
	talkEnd();

	_talkLine.drawString(string, 5, 0, 7, &_talkLineDim);

	_hasTalk = true;
	redrawScreen(_talkLineDim);
}

void Graphics::talkEnd() {
	_talkLine.clear();

	_hasTalk = false;
	redrawScreen(_talkLineDim);
}

void Graphics::blitToScreen(const Sprite &from,
		uint32 left, uint32 top, uint32 right, uint32 bottom,
		uint32 x, uint32 y, bool transp) {

	_screen.blit(from, left, top, right, bottom, x, y, transp);

	dirtyRectsAdd(x, y, x + (right - left), y + (bottom - top));
}

void Graphics::blitToScreen(const Sprite &from, uint32 x, uint32 y, bool transp) {
	_screen.blit(from, x, y, transp);

	dirtyRectsAdd(x, y, x + from.getWidth() - 1, y + from.getHeight() - 1);
}

void Graphics::retrace() {
	if (dirtyRectsApply())
		g_system->updateScreen();
}

bool Graphics::loadPAL(Common::SeekableReadStream &pal,
		int fromStart, int toStart, int count) {

	if (pal.size() < ((fromStart + count) * 4))
		return false;

	if (!pal.seek(fromStart * 4))
		return false;

	byte *palette = _gamePalette + toStart * 3;


	for (int i = 0; i < count; i++, palette += 3) {
		palette[2] = pal.readByte();
		palette[1] = pal.readByte();
		palette[0] = pal.readByte();

		pal.readByte();
	}

	applyGamePalette();

	return true;
}

bool Graphics::loadPAL(const Resource &resource,
		int fromStart, int toStart, int count) {

	return loadPAL(resource.getStream(), fromStart, toStart, count);
}

void Graphics::dirtyAll() {
	_dirtyAll = true;
	_dirtyRects.clear();
}

void Graphics::dirtyRectsAdd(const Common::Rect &rect) {
	if (_dirtyAll)
		return;

	if (_dirtyRects.size() >= 30)
		dirtyAll();

	_dirtyRects.push_back(rect);
}

void Graphics::dirtyRectsAdd(uint32 left, uint32 top, uint32 right, uint32 bottom) {
	dirtyRectsAdd(Common::Rect(left, top, right + 1, bottom + 1));
}

bool Graphics::dirtyRectsApply() {
	if (_dirtyAll) {
			g_system->copyRectToScreen(_screen.getData(), _screen.getWidth(),
					0, 0, _screen.getWidth(), _screen.getHeight());
		_dirtyAll = false;
		return true;
	}

	if (_dirtyRects.empty())
		return false;

	int screenWidth  = _screen.getWidth();
	int screenHeight = _screen.getHeight();

	Common::List<Common::Rect>::const_iterator it;
	for (it = _dirtyRects.begin(); it != _dirtyRects.end(); ++it) {
		int left   = MAX<int>(0, it->left);
		int top    = MAX<int>(0, it->top);
		int right  = MIN<int>(screenWidth , it->right);
		int bottom = MIN<int>(screenHeight, it->bottom);
		int width  = right  - left;
		int height = bottom - top;

		if ((width <= 0) || (height <= 0))
			continue;

		const byte *data = _screen.getData() + top * screenWidth + left;

		g_system->copyRectToScreen(data, screenWidth, left, top, width, height);
	}

	_dirtyRects.clear();
	return true;
}

void Graphics::registerBackground(Sprite &background) {
	_background = &background;

	redrawScreen(0, 0, _background->getWidth() - 1, _background->getHeight() - 1);
}

void Graphics::unregisterBackground() {
	_background = 0;
}

void Graphics::redrawScreen(const Common::Rect &rect) {
	redrawScreen(rect.left, rect.top, rect.right - 1, rect.bottom - 1);
}

void Graphics::redrawScreen(uint32 left, uint32 top, uint32 right, uint32 bottom) {
	blitToScreen(*_background, left, top, right, bottom, left, top, false);

	if (_hasTalk)
		blitToScreen(_talkLine, left, top, right, bottom, left, top, true);

	dirtyRectsAdd(left, top, right, bottom);
}

} // End of namespace DarkSeed2
