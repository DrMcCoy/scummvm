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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

// Decoder based on http://www.romhacking.net/docs/timgfx.txt

#include "darkseed2/tim.h"

#include "common/stream.h"
#include "common/textconsole.h"
#include "graphics/pixelformat.h"
#include "graphics/surface.h"

namespace DarkSeed2 {

TIMDecoder::TIMDecoder() {
	_surface = 0;
	_palette = 0;
	_paletteColorCount = 0;
}

TIMDecoder::~TIMDecoder() {
	destroy();
}

void TIMDecoder::destroy() {
	if (_surface) {
		_surface->free();
		delete _surface;
		_surface = 0;
	}

	delete[] _palette;
	_palette = 0;

	_paletteColorCount = 0;
}

bool TIMDecoder::loadStream(Common::SeekableReadStream &stream) {
	destroy();

	uint32 tag = stream.readUint32LE();
	uint32 version = stream.readUint32LE();

	if (stream.eos())
		return false;

	if (tag != 0x10)
		return false;

	switch (version) {
	case 0: // 4bpp (without CLUT)
		warning("Unhandled TIM 4bpp (without CLUT)");
		return false;
	case 1: // 8bpp (without CLUT)
		warning("Unhandled TIM 8bpp (without CLUT)");
		return false;
	case 2: // 16bpp
		return decodeTIM16(stream);
	case 3: // 24bpp
		return decodeTIM24(stream);
	case 8: // 4bpp (with CLUT)
		return decodeTIM4(stream);
	case 9: // 8bpp (with CLUT)
		return decodeTIM8(stream);
	}

	return false;
}

bool TIMDecoder::readPalette(Common::SeekableReadStream &stream, uint maxPaletteSize) {
	/* uint32 clutSize = */ stream.readUint32LE();
	/* uint16 palOrigX = */ stream.readUint16LE();
	/* uint16 palOrigY = */ stream.readUint16LE();
	uint16 colorCount = stream.readUint16LE();
	uint16 clutCount = stream.readUint16LE();

	if (clutCount != 1) {
		warning("Unsupported TIM CLUT count %d", clutCount);
		return false;
	}

	if (colorCount > maxPaletteSize) {
		warning("Invalid CLUT color count %d (max %d)", colorCount, maxPaletteSize);
		return false;
	}

	// Read in the palette, converting to RGB888

	Graphics::PixelFormat format(getFormat16());
	_paletteColorCount = colorCount;
	_palette = new byte[colorCount * 3];
	byte *pal = _palette;

	for (uint16 i = 0; i < colorCount; i++) {
		uint16 color = stream.readUint16LE();
		format.colorToRGB(color, pal[0], pal[1], pal[2]);
		pal += 3;
	}

	return true;
}

bool TIMDecoder::decodeTIM4(Common::SeekableReadStream &stream) {
	if (!readPalette(stream, 16))
		return false;

	/* uint32 fileSize = */ stream.readUint32LE();
	/* uint16 origX = */ stream.readUint16LE();
	/* uint16 origY = */ stream.readUint16LE();
	uint16 width = stream.readUint16LE() * 4;
	uint16 height = stream.readUint16LE();

	// Read in, converting to 8bpp

	_surface = new Graphics::Surface();
	_surface->create(width, height, Graphics::PixelFormat::createFormatCLUT8());
	byte *ptr = (byte *)_surface->getPixels();

	for (uint32 i = 0; i < (uint32)(width * height); i++) {
		byte val = stream.readByte();
		*ptr++ = val >> 4;
		*ptr++ = val & 0xF;
	}

	return true;
}

bool TIMDecoder::decodeTIM8(Common::SeekableReadStream &stream) {
	if (!readPalette(stream, 256))
		return false;

	/* uint32 fileSize = */ stream.readUint32LE();
	/* uint16 origX = */ stream.readUint16LE();
	/* uint16 origY = */ stream.readUint16LE();
	uint16 width = stream.readUint16LE() * 2;
	uint16 height = stream.readUint16LE();

	// Read in directly

	_surface = new Graphics::Surface();
	_surface->create(width, height, Graphics::PixelFormat::createFormatCLUT8());
	stream.read(_surface->getPixels(), width * height);

	return true;
}

bool TIMDecoder::decodeTIM16(Common::SeekableReadStream &stream) {
	/* uint32 fileSize = */ stream.readUint32LE();
	/* uint16 origX = */ stream.readUint16LE();
	/* uint16 origY = */ stream.readUint16LE();
	uint16 width = stream.readUint16LE() * 2 / 3;
	uint16 height = stream.readUint16LE();

	// Read in the BGR555 data

	_surface = new Graphics::Surface();
	_surface->create(width, height, getFormat16());
	uint16 *ptr = (uint16 *)_surface->getPixels();

	for (uint32 i = 0; i < (uint32)(width * height); i++)
		*ptr++ = stream.readUint16LE();

	return true;
}

bool TIMDecoder::decodeTIM24(Common::SeekableReadStream &stream) {
	/* uint32 fileSize = */ stream.readUint32LE();
	/* uint16 origX = */ stream.readUint16LE();
	/* uint16 origY = */ stream.readUint16LE();
	uint16 width = stream.readUint16LE() * 2 / 3;
	uint16 height = stream.readUint16LE();

	// Read in the BGR888 data, converting to a 32bpp format

	_surface = new Graphics::Surface();
	_surface->create(width, height, getFormat24());
	uint32 *ptr = (uint32 *)_surface->getPixels();

	for (uint16 y = 0; y < height; y++) {
		for (uint16 x = 0; x < width; x++) {
			byte b = stream.readByte();
			byte g = stream.readByte();
			byte r = stream.readByte();
			*ptr++ = _surface->format.RGBToColor(r, g, b);
		}
	}

	return true;
}

Graphics::PixelFormat TIMDecoder::getFormat16() {
	// BGR555
	return Graphics::PixelFormat(2, 5, 5, 5, 0, 0, 5, 10, 0);
}

Graphics::PixelFormat TIMDecoder::getFormat24() {
	// RGBX8880
	return Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0);
}

} // End of namespace DarkSeed2
