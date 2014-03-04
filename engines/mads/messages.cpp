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

#include "common/scummsys.h"
#include "mads/mads.h"
#include "mads/font.h"
#include "mads/graphics.h"
#include "mads/messages.h"
#include "mads/scene_data.h"

namespace MADS {

KernelMessages::KernelMessages(MADSEngine *vm): _vm(vm) {
	Scene &scene = _vm->_game->_scene;

	for (int i = 0; i < KERNEL_MESSAGES_SIZE; ++i) {
		KernelMessage rec;
		_entries.push_back(rec);
	}

	scene._textSpacing = -1;
	_talkFont = _vm->_font->getFont(FONT_CONVERSATION);
	word_8469E = 0;
}

void KernelMessages::clear() {
	Scene &scene = _vm->_game->_scene;

	for (uint i = 0; i < _entries.size(); ++i)
		_entries[i]._flags = 0;

	scene._textSpacing = -1;
	_talkFont = _vm->_font->getFont(FONT_CONVERSATION);
}

int KernelMessages::add(const Common::Point &pt, uint fontColor, uint8 flags, 
		uint8 abortTimers, uint32 timeout, const Common::String &msg) {
	Scene &scene = _vm->_game->_scene; 

	// Find a free slot
	uint idx = 0;
	while ((idx < _entries.size()) && ((_entries[idx]._flags & KMSG_ACTIVE) != 0))
		++idx;
	if (idx == _entries.size()) {
		if (abortTimers == 0)
			return -1;

		error("KernelMessages overflow");
	}

	KernelMessage &rec = _entries[idx];
	rec._msg = msg;
	rec._flags = flags | KMSG_ACTIVE;
	rec._color1 = fontColor & 0xff;
	rec._color2 = fontColor >> 8;
	rec._position = pt;
	rec._textDisplayIndex = -1;
	rec._timeout = timeout;
	rec._frameTimer = _vm->_game->_priorFrameTimer;
	rec._abortTimers = abortTimers;
	rec._abortMode = _vm->_game->_abortTimersMode2;

	rec._actionDetails = scene._action._activeAction;

	if (flags & KMSG_PLAYER_TIMEOUT)
		rec._frameTimer = _vm->_game->_player._ticksAmount + 
			_vm->_game->_player._priorTimer;

	return idx;
}

int KernelMessages::addQuote(int quoteId, int abortTimers, uint32 timeout) {
	Common::String quoteStr = _vm->_game->getQuote(quoteId);
	return add(Common::Point(), 0x1110, KMSG_PLAYER_TIMEOUT | KMSG_CENTER_ALIGN, abortTimers, timeout, quoteStr);
}

void KernelMessages::scrollMessage(int msgIndex, int numTicks, bool quoted) {
	if (msgIndex < 0)
		return;

	_entries[msgIndex]._flags |= quoted ? (KMSG_SCROLL | KMSG_QUOTED) : KMSG_SCROLL;
	_entries[msgIndex]._msgOffset = 0;
	_entries[msgIndex]._numTicks = numTicks;
	_entries[msgIndex]._frameTimer2 = _vm->_game->_priorFrameTimer;

	Common::String msg = _entries[msgIndex]._msg;
	_entries[msgIndex]._asciiChar = msg[0];
	_entries[msgIndex]._asciiChar2 = msg[1];

	if (_entries[msgIndex]._flags & KMSG_PLAYER_TIMEOUT)
		_entries[msgIndex]._frameTimer2 = _vm->_game->_player._ticksAmount + 
		_vm->_game->_player._priorTimer;

	_entries[msgIndex]._frameTimer = _entries[msgIndex]._frameTimer2;
}

void KernelMessages::setSeqIndex(int msgIndex, int seqIndex) {
	if (msgIndex >= 0) {
		_entries[msgIndex]._flags |= KMSG_SEQ_ENTRY;
		_entries[msgIndex]._sequenceIndex = seqIndex;
	}
}

void KernelMessages::remove(int msgIndex) {
	KernelMessage &rec = _entries[msgIndex];
	Scene &scene = _vm->_game->_scene;

	if (rec._flags & KMSG_ACTIVE) {
		if (rec._flags & KMSG_SCROLL) {
			rec._msg.setChar(rec._asciiChar, rec._msgOffset);
			rec._msg.setChar(rec._asciiChar2, rec._msgOffset + 1);
		}

		if (rec._textDisplayIndex >= 0)
			scene._textDisplay.expire(rec._textDisplayIndex);

		rec._flags &= ~KMSG_ACTIVE;
	}
}

void KernelMessages::reset() {
	for (uint i = 0; i < _entries.size(); ++i)
		remove(i);

	warning("TODO: KernelMessages::reset - sub_20454");
}

void KernelMessages::update() {
	uint32 currentTimer = _vm->_game->_priorFrameTimer;

	for (uint i = 0; i < _entries.size(); ++i) {
		if (((_entries[i]._flags & KMSG_ACTIVE) != 0) && 
				(currentTimer >= _entries[i]._frameTimer))
			processText(i);
	}
}

void KernelMessages::processText(int msgIndex) {
	Scene &scene = _vm->_game->_scene;
	KernelMessage &msg = _entries[msgIndex];
	uint32 currentTimer = _vm->_game->_priorFrameTimer;
	bool flag = false;

	if ((msg._flags & KMSG_EXPIRE) != 0) {
		scene._textDisplay.expire(msg._textDisplayIndex);
		msg._flags &= !KMSG_ACTIVE;
		return;
	}

	if ((msg._flags & KMSG_SCROLL) == 0) {
		msg._timeout -= 3;
	}

	if (msg._flags & KMSG_SEQ_ENTRY) {
		SequenceEntry &seqEntry = scene._sequences[msg._sequenceIndex];
		if (seqEntry._doneFlag || !seqEntry._active)
			msg._timeout = 0;
	}

	if ((msg._timeout <= 0) && (_vm->_game->_abortTimers == 0)) {
		msg._flags |= KMSG_EXPIRE;
		if (msg._abortTimers != 0) {
			_vm->_game->_abortTimers = msg._abortTimers;
			_vm->_game->_abortTimersMode = msg._abortMode;

			if (_vm->_game->_abortTimersMode != ABORTMODE_1) {
				scene._action._activeAction = msg._actionDetails;
			}
		}
	}

	msg._frameTimer = currentTimer + 3;
	int x1 = 0, y1 = 0;

	if (msg._flags & KMSG_SEQ_ENTRY) {
		SequenceEntry &seqEntry = scene._sequences[msg._sequenceIndex];
		if (!seqEntry._nonFixed) {
			SpriteAsset &spriteSet = scene._spriteSlots.getSprite(seqEntry._spritesIndex);
			MSprite *frame = spriteSet.getFrame(seqEntry._frameIndex - 1);
			x1 = frame->getBounds().left;
			y1 = frame->getBounds().top;
		} else {
			x1 = seqEntry._msgPos.x;
			y1 = seqEntry._msgPos.y;
		}
	}

	if (msg._flags & KMSG_PLAYER_TIMEOUT) {
		if (word_8469E != 0) {
			warning("TODO: KernelMessages::processText");
			// TODO: Figure out various flags
		} else {
			x1 = 160;
			y1 = 78;
		}
	}

	x1 += msg._position.x;
	y1 += msg._position.y;

	if ((msg._flags & KMSG_SCROLL) && (msg._frameTimer >= currentTimer)) {
		msg._msg.setChar(msg._asciiChar, msg._msgOffset);

		++msg._msgOffset;
		msg._msg.setChar(msg._asciiChar2, msg._msgOffset);
		msg._asciiChar = msg._msg[msg._msgOffset];
		msg._asciiChar2 = msg._msg[msg._msgOffset + 1];

		if (!msg._asciiChar) {
			// End of message
			msg._msg.setChar('\0', msg._msgOffset);
			msg._flags &= ~KMSG_SCROLL;
		} else if (msg._flags & KMSG_QUOTED) {
			msg._msg.setChar('"', msg._msgOffset);
			msg._msg.setChar('\0', msg._msgOffset + 1);
		}

		msg._frameTimer = msg._frameTimer2 = currentTimer + msg._numTicks;
		flag = true;
	}

	int strWidth = _talkFont->getWidth(msg._msg, scene._textSpacing);

	if (msg._flags & (KMSG_RIGHT_ALIGN | KMSG_CENTER_ALIGN)) {
		x1 -= (msg._flags & KMSG_CENTER_ALIGN) ? strWidth / 2 : strWidth;
	}

	// Make sure text appears entirely on-screen
	int x2 = x1 + strWidth;
	if (x2 > MADS_SCREEN_WIDTH)
		x1 -= x2 - MADS_SCREEN_WIDTH;
	if (x1 > (MADS_SCREEN_WIDTH - 1))
		x1 = MADS_SCREEN_WIDTH - 1;
	if (x1 < 0)
		x1 = 0;

	if (y1 >(MADS_SCENE_HEIGHT - 1))
		y1 = MADS_SCENE_HEIGHT - 1;
	if (y1 < 0)
		y1 = 0;

	if (msg._textDisplayIndex >= 0) {
		TextDisplay &textEntry = scene._textDisplay[msg._textDisplayIndex];

		if (flag || (textEntry._bounds.left != x1) || (textEntry._bounds.top != y1)) {
			// Mark the associated text entry as deleted, so it can be re-created
			scene._textDisplay.expire(msg._textDisplayIndex);
			msg._textDisplayIndex = -1;
		}
	}

	if (msg._textDisplayIndex < 0) {
		// Need to create a new text display entry for this message
		int idx = scene._textDisplay.add(x1, y1, msg._color1 | (msg._color2 << 8), 
			scene._textSpacing, msg._msg, _talkFont);
		if (idx >= 0)
			msg._textDisplayIndex = idx;
	}
}

void KernelMessages::delay(uint32 priorFrameTime, uint32 currentTime) {
	for (uint i = 0; i < _entries.size(); ++i) {
		_entries[i]._timeout += currentTime - priorFrameTime;
	}
}

/*------------------------------------------------------------------------*/

TextDisplay::TextDisplay() {
	_active = false;
	_expire = 0;
	_spacing = 0;
	_color1 = 0;
	_color2 = 0;
	_font = nullptr;
}

/*------------------------------------------------------------------------*/

TextDisplayList::TextDisplayList(MADSEngine *vm) : _vm(vm) {
	for (int i = 0; i < TEXT_DISPLAY_SIZE; ++i) {
		TextDisplay rec;
		rec._active = false;
		rec._expire = 0;
		_entries.push_back(rec);
	}
}

void TextDisplayList::clear() {
	for (int i = 0; i < TEXT_DISPLAY_SIZE; ++i)
		_entries[i]._active = false;
}

int TextDisplayList::add(int xp, int yp, uint fontColor, int charSpacing, 
		const Common::String &msg, Font *font) {
	int usedSlot = -1;

	for (int idx = 0; idx < TEXT_DISPLAY_SIZE; ++idx) {
		if (!_entries[idx]._active) {
			usedSlot = idx;

			_entries[idx]._bounds.left = xp;
			_entries[idx]._bounds.top = yp;
			_entries[idx]._font = font;
			_entries[idx]._msg = msg;
			_entries[idx]._bounds.setWidth(font->getWidth(msg, charSpacing));
			_entries[idx]._bounds.setHeight(font->getHeight());
			_entries[idx]._color1 = fontColor & 0xff;
			_entries[idx]._color2 = fontColor >> 8;
			_entries[idx]._spacing = charSpacing;
			_entries[idx]._expire = 1;
			_entries[idx]._active = true;
			break;
		}
	}

	return usedSlot;
}

void TextDisplayList::setDirtyAreas() {
	Scene &scene = _vm->_game->_scene;

	for (uint idx = 0, dirtyIdx = DIRTY_AREAS_TEXT_DISPLAY_IDX; dirtyIdx < DIRTY_AREAS_SIZE; ++idx, ++dirtyIdx) {
		if ((_entries[idx]._expire >= 0) || !_entries[idx]._active)
			scene._dirtyAreas[dirtyIdx]._active = false;
		else {
			scene._dirtyAreas[dirtyIdx]._textActive = true;
			scene._dirtyAreas.setTextDisplay(dirtyIdx, _entries[idx]);
		}
	}
}

void TextDisplayList::setDirtyAreas2() {
	Scene &scene = _vm->_game->_scene;

	for (uint idx = 0, dirtyIdx = DIRTY_AREAS_TEXT_DISPLAY_IDX; dirtyIdx < DIRTY_AREAS_SIZE; ++idx, ++dirtyIdx) {
		if (_entries[idx]._active && (_entries[idx]._expire >= 0)) {
			scene._dirtyAreas.setTextDisplay(dirtyIdx, _entries[idx]);
			scene._dirtyAreas[dirtyIdx]._textActive = (_entries[idx]._expire <= 0) ? 0 : 1;
		}
	}
}

void TextDisplayList::draw(MSurface *view) {
	error("TODO");
	/*
	for (uint idx = 0; idx < _entries.size(); ++idx) {
		if (_entries[idx]._active && (_entries[idx]._expire >= 0)) {
			_entries[idx]._font->setColors(_entries[idx]._color1, _entries[idx]._color2, 0);
			_entries[idx]._font->writeString(view, _entries[idx]._msg,
				Common::Point(_entries[idx]._bounds.left, _entries[idx]._bounds.top), 
				_entries[idx]._bounds.width(), _entries[idx]._spacing);
		}
	}
	*/
}

void TextDisplayList::cleanUp() {
	for (uint idx = 0; idx < _entries.size(); ++idx) {
		if (_entries[idx]._expire < 0) {
			_entries[idx]._active = false;
			_entries[idx]._expire = 0;
		}
	}
}

void TextDisplayList::expire(int idx) {
	_entries[idx]._expire = -1;
}

} // End of namespace MADS
