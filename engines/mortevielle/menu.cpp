/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file _distributed with this source _distribution.
 *
 * This program is free software; you can re_distribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is _distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on original Mortville Manor DOS source code
 * Copyright (c) 1987-1989 Lankhor
 */

#include "mortevielle/mortevielle.h"

#include "mortevielle/menu.h"
#include "mortevielle/mouse.h"
#include "mortevielle/outtext.h"

#include "common/scummsys.h"
#include "common/str.h"
#include "common/textconsole.h"

namespace Mortevielle {

const byte menuConstants[8][4] = {
	{ 7, 37, 23,  8},
	{19, 33, 23,  7},
	{31, 89, 10, 21},
	{43, 25, 11,  5},
	{55, 37,  5,  8},
	{64, 13, 11,  2},
	{62, 42, 13,  9},
	{62, 46, 13, 10}
};

Menu::Menu() {
	OPCODE_ATTACH = OPCODE_WAIT = OPCODE_FORCE = OPCODE_SLEEP = OPCODE_NONE;
	OPCODE_LISTEN = OPCODE_ENTER = OPCODE_CLOSE = OPCODE_SEARCH = OPCODE_NONE;
	OPCODE_KNOCK = OPCODE_SCRATCH = OPCODE_READ = OPCODE_EAT = OPCODE_NONE;
	OPCODE_PLACE = OPCODE_OPEN = OPCODE_TAKE = OPCODE_LOOK = OPCODE_NONE;
	OPCODE_SMELL = OPCODE_SOUND = OPCODE_LEAVE = OPCODE_LIFT = OPCODE_NONE;
	OPCODE_TURN = OPCODE_SHIDE = OPCODE_SSEARCH = OPCODE_SREAD = OPCODE_NONE;
	OPCODE_SPUT = OPCODE_SLOOK = OPCODE_NONE;
}

void Menu::readVerbNums(Common::File &f, int dataSize) {
	// Figure out what language Id is needed
	byte desiredLanguageId;
	switch(_vm->getLanguage()) {
	case Common::EN_ANY:
		desiredLanguageId = MORTDAT_LANG_ENGLISH;
		break;
	case Common::FR_FRA:
		desiredLanguageId = MORTDAT_LANG_FRENCH;
		break;
	case Common::DE_DEU:
		desiredLanguageId = MORTDAT_LANG_GERMAN;
		break;
	default:
		warning("Language not supported, switching to English");
		desiredLanguageId = MORTDAT_LANG_ENGLISH;
		break;
	}
	// Read in the language
	byte languageId = f.readByte();
	--dataSize;

	// If the language isn't correct, then skip the entire block
	if (languageId != desiredLanguageId) {
		f.skip(dataSize);
		return;
	}

	assert(dataSize == 52);
	OPCODE_ATTACH  = f.readUint16LE();
	OPCODE_WAIT    = f.readUint16LE();
	OPCODE_FORCE   = f.readUint16LE();
	OPCODE_SLEEP   = f.readUint16LE();
	OPCODE_LISTEN  = f.readUint16LE();
	OPCODE_ENTER   = f.readUint16LE();
	OPCODE_CLOSE   = f.readUint16LE();
	OPCODE_SEARCH  = f.readUint16LE();
	OPCODE_KNOCK   = f.readUint16LE();
	OPCODE_SCRATCH = f.readUint16LE();
	OPCODE_READ    = f.readUint16LE();
	OPCODE_EAT     = f.readUint16LE();
	OPCODE_PLACE   = f.readUint16LE();
	OPCODE_OPEN    = f.readUint16LE();
	OPCODE_TAKE    = f.readUint16LE();
	OPCODE_LOOK    = f.readUint16LE();
	OPCODE_SMELL   = f.readUint16LE();
	OPCODE_SOUND   = f.readUint16LE();
	OPCODE_LEAVE   = f.readUint16LE();
	OPCODE_LIFT    = f.readUint16LE();
	OPCODE_TURN    = f.readUint16LE();
	OPCODE_SHIDE   = f.readUint16LE();
	OPCODE_SSEARCH = f.readUint16LE();
	OPCODE_SREAD   = f.readUint16LE();
	OPCODE_SPUT    = f.readUint16LE();
	OPCODE_SLOOK   = f.readUint16LE();

	_actionMenu[0]._menuId   = OPCODE_NONE   >> 8;
	_actionMenu[0]._actionId = OPCODE_NONE   & 0xFF;

	_actionMenu[1]._menuId   = OPCODE_SHIDE  >> 8;
	_actionMenu[1]._actionId = OPCODE_SHIDE  & 0xFF;

	_actionMenu[2]._menuId   = OPCODE_ATTACH >> 8;
	_actionMenu[2]._actionId = OPCODE_ATTACH & 0xFF; 

	_actionMenu[3]._menuId   = OPCODE_FORCE  >> 8;
	_actionMenu[3]._actionId = OPCODE_FORCE  & 0xFF; 

	_actionMenu[4]._menuId   = OPCODE_SLEEP  >> 8;
	_actionMenu[4]._actionId = OPCODE_SLEEP  & 0xFF;

	_actionMenu[5]._menuId   = OPCODE_ENTER  >> 8;
	_actionMenu[5]._actionId = OPCODE_ENTER  & 0xFF; 

	_actionMenu[6]._menuId   = OPCODE_CLOSE  >> 8;
	_actionMenu[6]._menuId   = OPCODE_CLOSE  & 0xFF; 

	_actionMenu[7]._menuId   = OPCODE_KNOCK  >> 8;
	_actionMenu[7]._menuId   = OPCODE_KNOCK  & 0xFF;

	_actionMenu[8]._menuId   = OPCODE_EAT    >> 8;
	_actionMenu[8]._menuId   = OPCODE_EAT    & 0xFF;

	_actionMenu[9]._menuId   = OPCODE_PLACE  >> 8;
	_actionMenu[9]._menuId   = OPCODE_PLACE  & 0xFF;

	_actionMenu[10]._menuId  = OPCODE_OPEN   >> 8;
	_actionMenu[10]._menuId  = OPCODE_OPEN   & 0xFF;

	_actionMenu[11]._menuId  = OPCODE_LEAVE  >> 8;
	_actionMenu[11]._menuId  = OPCODE_LEAVE  & 0xFF;
}

/**
 * Setup a menu's contents
 * @remarks	Originally called 'menut'
 */
void Menu::setText(int menuId, int actionId, Common::String name) {
	Common::String s = name;

	while (s.size() < 22)
		s += ' ';

	switch (menuId) {
	case MENU_INVENTORY:
		if (actionId != 7) {
			_inventoryStringArray[actionId] = s;
			_inventoryStringArray[actionId].insertChar(' ', 0);
		}
		break;
	case MENU_MOVE:
		_moveStringArray[actionId] = s;
		break;
	case MENU_ACTION:
		_actionStringArray[actionId] = s;
		break;
	case MENU_SELF:
		_selfStringArray[actionId] = s;
		break;
	case MENU_DISCUSS:
		_discussStringArray[actionId] = s;
		break;
	default:
		break;
	}
}

/**
 * Init destination menu
 * @remarks	Originally called 'tmlieu'
 */
void Menu::setDestinationText(int roomId) {
	Common::String nomp;

	if (roomId == 26)
		roomId = LANDING;

	int destinationId = 0;
	for (; (destinationId < 7) && (_vm->_destinationArray[destinationId][roomId]); ++destinationId) {
		nomp = _vm->getString(_vm->_destinationArray[destinationId][roomId] + kMenuPlaceStringIndex);
		while (nomp.size() < 20)
			nomp += ' ';
		setText(_moveMenu[destinationId + 1]._menuId, _moveMenu[destinationId + 1]._actionId, nomp);
	}
	nomp = "*                   ";
	for (int i = 7; i >= destinationId + 1; --i)
		setText(_moveMenu[i]._menuId, _moveMenu[i]._actionId, nomp);
}

/**
 * _disable a menu item
 * @param menuId	Menu number
 * @param actionId  Item index
 */
void Menu::disableMenuItem(int menuId, int actionId) {
	switch (menuId) {
	case MENU_INVENTORY:
		if (actionId > 6) {
			_inventoryStringArray[actionId].setChar('<', 0);
			_inventoryStringArray[actionId].setChar('>', 21);
		} else
			_inventoryStringArray[actionId].setChar('*', 0);
		break;
	case MENU_MOVE:
		_moveStringArray[actionId].setChar('*', 0);
		break;
	case MENU_ACTION:
		_actionStringArray[actionId].setChar('*', 0);
		break;
	case MENU_SELF:
		_selfStringArray[actionId].setChar('*', 0);
		break;
	case MENU_DISCUSS:
		_discussStringArray[actionId].setChar('*', 0);
		break;
	default:
		break;
	}
}

/**
 * Enable a menu item
 * @param menuId	Menu number
 * @param actionId  Item index
 * @remarks	Originally called menu_enable
 */
void Menu::enableMenuItem(int menuId, int actionId) {
	switch (menuId) {
	case MENU_INVENTORY:
		_inventoryStringArray[actionId].setChar(' ', 0);
		_inventoryStringArray[actionId].setChar(' ', 21);
		break;
	case MENU_MOVE:
		_moveStringArray[actionId].setChar(' ', 0);
		break;
	case MENU_ACTION:
		_actionStringArray[actionId].setChar(' ', 0);
		break;
	case MENU_SELF:
		_selfStringArray[actionId].setChar(' ', 0);
		// The original sets two times the same value. Skipped
		// _selfStringArray[l].setChar(' ', 0);
		break;
	case MENU_DISCUSS:
		_discussStringArray[actionId].setChar(' ', 0);
		break;
	default:
		break;
	}
}

void Menu::displayMenu() {
	int ind_tabl, k, col;

	int pt, x, y, color, msk, num_letr;

	_vm->_mouse.hideMouse();

	_vm->_screenSurface.fillRect(7, Common::Rect(0, 0, 639, 10));
	col = 28 * _vm->_resolutionScaler;
	if (_vm->_currGraphicalDevice == MODE_CGA)
		color = 1;
	else
		color = 9;
	num_letr = 0;
	do {       // One character after the other
		++num_letr;
		ind_tabl = 0;
		y = 1;
		do {     // One column after the other
			k = 0;
			x = col;
			do {   // One line after the other
				msk = 0x80;
				for (pt = 0; pt <= 7; ++pt) {
					if ((_charArr[num_letr - 1][ind_tabl] & msk) != 0) {
						_vm->_screenSurface.setPixel(Common::Point(x + 1, y + 1), 0);
						_vm->_screenSurface.setPixel(Common::Point(x, y + 1), 0);
						_vm->_screenSurface.setPixel(Common::Point(x, y), color);
					}
					msk = (uint)msk >> 1;
					++x;
				}
				++ind_tabl;
				++k;
			} while (k != 3);
			++y;
		} while (y != 9);
		col += 48 * _vm->_resolutionScaler;
	} while (num_letr != 6);
	_vm->_mouse.showMouse();
}

/**
 * Show the menu
 */
void Menu::drawMenu() {
	displayMenu();
	_menuActive = true;
	_msg4 = OPCODE_NONE;
	_msg3 = OPCODE_NONE;
	_menuSelected = false;
	_vm->setMouseClick(false);
	_multiTitle = false;
}

/**
 * Menu function - Invert a menu entry
 * @remarks	Originally called 'invers'
 */
void Menu::invert(int indx) {
	if (_msg4 == OPCODE_NONE)
		return;

	int menuIndex = _msg4 & 0xFF;

	_vm->_screenSurface.putxy(menuConstants[_msg3 - 1][0] << 3, (menuIndex + 1) << 3);

	Common::String str;
	switch (_msg3) {
	case MENU_INVENTORY:
		str = _inventoryStringArray[menuIndex];
		break;
	case MENU_MOVE:
		str = _moveStringArray[menuIndex];
		break;
	case MENU_ACTION:
		str = _actionStringArray[menuIndex];
		break;
	case MENU_SELF:
		str = _selfStringArray[menuIndex];
		break;
	case MENU_DISCUSS:
		str = _discussStringArray[menuIndex];
		break;
	case MENU_FILE:
		str = _vm->getEngineString(S_SAVE_LOAD + menuIndex);
		break;
	case MENU_SAVE:
		str = _vm->getEngineString(S_SAVE_LOAD + 1);
		str += ' ';
		str += (char)(48 + menuIndex);
		break;
	case MENU_LOAD:
		if (menuIndex == 1) {
			str = _vm->getEngineString(S_RESTART);
		} else {
			str = _vm->getEngineString(S_SAVE_LOAD + 2);
			str += ' ';
			str += (char)(47 + menuIndex);
		}
		break;
	default:
		break;
	}
	if ((str[0] != '*') && (str[0] != '<'))
		_vm->_screenSurface.drawString(str, indx);
	else
		_msg4 = OPCODE_NONE;
}

void Menu::util(Common::Point pos) {

	int ymx = (menuConstants[_msg3 - 1][3] << 3) + 16;
	int dxcar = menuConstants[_msg3 - 1][2];
	int xmn = (menuConstants[_msg3 - 1][0] << 2) * _vm->_resolutionScaler;

	int ix;
	if (_vm->_resolutionScaler == 1)
		ix = 5;
	else
		ix = 3;
	int xmx = dxcar * ix * _vm->_resolutionScaler + xmn + 2;
	if ((pos.x > xmn) && (pos.x < xmx) && (pos.y < ymx) && (pos.y > 15)) {
		ix = (((uint)pos.y >> 3) - 1) + (_msg3 << 8);
		if (ix != _msg4) {
			invert(1);
			_msg4 = ix;
			invert(0);
		}
	} else if (_msg4 != OPCODE_NONE) {
		invert(1);
		_msg4 = OPCODE_NONE;
	}
}

/**
 * Draw a menu
 */
void Menu::menuDown(int ii) {
	int cx, xcc, xco;
	int lignNumb;

	// Make a copy of the current screen surface for later restore
	_vm->_backgroundSurface.copyFrom(_vm->_screenSurface);

	// Draw the menu
	xco = menuConstants[ii - 1][0];
	lignNumb = menuConstants[ii - 1][3];
	_vm->_mouse.hideMouse();
	xco = xco << 3;
	if (_vm->_resolutionScaler == 1)
		cx = 10;
	else
		cx = 6;
	xcc = xco + (menuConstants[ii - 1][2] * cx) + 6;
	if ((ii == 4) && (_vm->getLanguage() == Common::EN_ANY))
		// Extra width needed for Self menu in English version
		xcc = 435;

	_vm->_screenSurface.fillRect(15, Common::Rect(xco, 12, xcc, 10 + (menuConstants[ii - 1][1] << 1)));
	_vm->_screenSurface.fillRect(0, Common::Rect(xcc, 12, xcc + 4, 10 + (menuConstants[ii - 1][1] << 1)));
	_vm->_screenSurface.fillRect(0, Common::Rect(xco, 8 + (menuConstants[ii - 1][1] << 1), xcc + 4, 12 + (menuConstants[ii - 1][1] << 1)));
	_vm->_screenSurface.putxy(xco, 16);
	cx = 0;
	do {
		++cx;
		switch (ii) {
		case 1:
			if (_inventoryStringArray[cx][0] != '*')
				_vm->_screenSurface.drawString(_inventoryStringArray[cx], 4);
			break;
		case 2:
			if (_moveStringArray[cx][0] != '*')
				_vm->_screenSurface.drawString(_moveStringArray[cx], 4);
			break;
		case 3:
			if (_actionStringArray[cx][0] != '*')
				_vm->_screenSurface.drawString(_actionStringArray[cx], 4);
			break;
		case 4:
			if (_selfStringArray[cx][0] != '*')
				_vm->_screenSurface.drawString(_selfStringArray[cx], 4);
			break;
		case 5:
			if (_discussStringArray[cx][0] != '*')
				_vm->_screenSurface.drawString(_discussStringArray[cx], 4);
			break;
		case 6:
			_vm->_screenSurface.drawString(_vm->getEngineString(S_SAVE_LOAD + cx), 4);
			break;
		case 7: {
			Common::String s = _vm->getEngineString(S_SAVE_LOAD + 1);
			s += ' ';
			s += (char)(48 + cx);
			_vm->_screenSurface.drawString(s, 4);
			}
			break;
		case 8:
			if (cx == 1)
				_vm->_screenSurface.drawString(_vm->getEngineString(S_RESTART), 4);
			else {
				Common::String s = _vm->getEngineString(S_SAVE_LOAD + 2);
				s += ' ';
				s += (char)(47 + cx);
				_vm->_screenSurface.drawString(s, 4);
			}
			break;
		default:
			break;
		}
		_vm->_screenSurface.putxy(xco, _vm->_screenSurface._textPos.y + 8);
	} while (cx != lignNumb);
	_multiTitle = true;
	_vm->_mouse.showMouse();
}

/**
 * Menu is being removed, so restore the previous background area.
 */
void Menu::menuUp(int msgId) {
	if (_multiTitle) {
		/* Restore the background area */
		assert(_vm->_screenSurface.pitch == _vm->_backgroundSurface.pitch);

		// Get a pointer to the source and destination of the area to restore
		const byte *pSrc = (const byte *)_vm->_backgroundSurface.getBasePtr(0, 10);
		Graphics::Surface destArea = _vm->_screenSurface.lockArea(Common::Rect(0, 10, SCREEN_WIDTH, SCREEN_HEIGHT));
		byte *pDest = (byte *)destArea.getPixels();

		// Copy the data
		Common::copy(pSrc, pSrc + (400 - 10) * SCREEN_WIDTH, pDest);

		_multiTitle = false;
	}
}

/**
 * Erase the menu
 */
void Menu::eraseMenu() {
	_menuActive = false;
	_vm->setMouseClick(false);
	menuUp(_msg3);
}

/**
 * Handle updates to the menu
 * @remarks	Originally called 'mdn'
 */
void Menu::updateMenu() {
	if (!_menuActive)
		return;

	Common::Point curPos = _vm->_mouse._pos;
	if (!_vm->getMouseClick()) {
		if (curPos == _vm->_prevPos)
			return;
		else
			_vm->_prevPos = curPos;

		bool tes =  (curPos.y < 11)
		   && ((curPos.x >= (28 * _vm->_resolutionScaler) && curPos.x <= (28 * _vm->_resolutionScaler + 24))
		   ||  (curPos.x >= (76 * _vm->_resolutionScaler) && curPos.x <= (76 * _vm->_resolutionScaler + 24))
		   || ((curPos.x > 124 * _vm->_resolutionScaler) && (curPos.x < 124 * _vm->_resolutionScaler + 24))
		   || ((curPos.x > 172 * _vm->_resolutionScaler) && (curPos.x < 172 * _vm->_resolutionScaler + 24))
		   || ((curPos.x > 220 * _vm->_resolutionScaler) && (curPos.x < 220 * _vm->_resolutionScaler + 24))
		   || ((curPos.x > 268 * _vm->_resolutionScaler) && (curPos.x < 268 * _vm->_resolutionScaler + 24)));
		if (tes) {
			int ix;

			if (curPos.x < 76 * _vm->_resolutionScaler)
				ix = MENU_INVENTORY;
			else if (curPos.x < 124 * _vm->_resolutionScaler)
				ix = MENU_MOVE;
			else if (curPos.x < 172 * _vm->_resolutionScaler)
				ix = MENU_ACTION;
			else if (curPos.x < 220 * _vm->_resolutionScaler)
				ix = MENU_SELF;
			else if (curPos.x < 268 * _vm->_resolutionScaler)
				ix = MENU_DISCUSS;
			else
				ix = MENU_FILE;

			if ((ix != _msg3) || (!_multiTitle))
				if (!((ix == MENU_FILE) && ((_msg3 == MENU_SAVE) || (_msg3 == MENU_LOAD)))) {
					menuUp(_msg3);
					menuDown(ix);
					_msg3 = ix;
					_msg4 = OPCODE_NONE;
				}
		} else { // Not in the MenuTitle line
			if ((curPos.y > 11) && (_multiTitle))
				util(curPos);
		}
	} else {       // There was a click
		if ((_msg3 == MENU_FILE) && (_msg4 != OPCODE_NONE)) {
			// Another menu to be _displayed
			_vm->setMouseClick(false);
			menuUp(_msg3);
			if ((_msg4 & 0xFF) == 1)
				_msg3 = MENU_SAVE;
			else
				_msg3 = MENU_LOAD;
			menuDown(_msg3);

			_vm->setMouseClick(false);
		} else {
			//  A menu was clicked on
			_menuSelected = (_multiTitle) && (_msg4 != OPCODE_NONE);
			menuUp(_msg3);
			_vm->_currAction = _msg4;
			_vm->_currMenu = _msg3;
			_msg3 = OPCODE_NONE;
			_msg4 = OPCODE_NONE;

			_vm->setMouseClick(false);
		}
	}
}

void Menu::setParent(MortevielleEngine *vm) {
	_vm = vm;
}

void Menu::initMenu() {
	int i;
	Common::File f;
	
	bool enMenuLoaded = false;
	if (_vm->getLanguage() == Common::EN_ANY) {
		// Open the mort.dat file
		if (!f.open(MORT_DAT))
			warning("File %s not found. Using default menu from game data", MORT_DAT);
		else {
			// Validate the data file header
			char fileId[4];
			f.read(fileId, 4);
			// Do not display warnings here. They would already have been displayed in MortevielleEngine::loadMortDat().
			if (strncmp(fileId, "MORT", 4) == 0 && f.readByte() >= MORT_DAT_REQUIRED_VERSION) {
				f.readByte();		// Minor version
				// Loop to load resources from the data file
				while (f.pos() < f.size()) {
					// Get the Id and size of the next resource
					char dataType[4];
					int dataSize;
					f.read(dataType, 4);
					dataSize = f.readUint16LE();
					if (!strncmp(dataType, "MENU", 4)) {
						// MENU section
						if (dataSize <= 7 * 24) {
							f.read(_charArr, dataSize);
							enMenuLoaded = true;
						} else
							warning("Wrong size %d for menu data. Expected %d or less", dataSize, 7*24);
						break;
					} else {
						// Other sections
						f.skip(dataSize);
					}
				}
			}
			// Close the file
			f.close();
			if (!enMenuLoaded)
				warning("Failed to load English menu. Will use default menu from game data instead");
		}
	}

	if (!enMenuLoaded) {
		if (!f.open("menufr.mor"))
			if (!f.open("menual.mor"))
				if (!f.open("menu.mor"))
					error("Missing file - menufr.mor or menual.mor or menu.mor");

		f.read(_charArr, 7 * 24);
		f.close();
	}

	// Skipped: dialog asking to swap floppy

	for (i = 1; i <= 8; ++i)
		_inventoryStringArray[i] = "*                     ";
	_inventoryStringArray[7] = "< -*-*-*-*-*-*-*-*-*- ";
	for (i = 1; i <= 7; ++i)
		_moveStringArray[i] = "*                       ";
	i = 1;
	do {
		_actionStringArray[i] = _vm->getString(i + kMenuActionStringIndex);

		while (_actionStringArray[i].size() < 10)
			_actionStringArray[i] += ' ';

		if (i < 9) {
			if (i < 6) {
				_selfStringArray[i] = _vm->getString(i + kMenuSelfStringIndex);
				while (_selfStringArray[i].size() < 10)
					_selfStringArray[i] += ' ';
			}
			_discussStringArray[i] = _vm->getString(i + kMenuSayStringIndex) + ' ';
		}
		++i;
	} while (i != 22);
	for (i = 1; i <= 8; ++i) {
		_discussMenu[i]._menuId = MENU_DISCUSS;
		_discussMenu[i]._actionId = i;
		if (i < 8) {
			_moveMenu[i]._menuId = MENU_MOVE;
			_moveMenu[i]._actionId = i;
		}
		_inventoryMenu[i]._menuId = MENU_INVENTORY;
		_inventoryMenu[i]._actionId = i;
		if (i > 6)
			disableMenuItem(_inventoryMenu[i]._menuId, _inventoryMenu[i]._actionId);
	}
	_msg3 = OPCODE_NONE;
	_msg4 = OPCODE_NONE;
	_vm->_currMenu = OPCODE_NONE;
	_vm->_currAction = OPCODE_NONE;
	_vm->setMouseClick(false);
}

/**
 * Engine function - Switch action menu to "Search" mode
 * @remarks	Originally called 'mfoudi'
 */
void Menu::setSearchMenu() {
	for (int i = 1; i <= 7; ++i)
		disableMenuItem(MENU_MOVE, _moveMenu[i]._actionId);

	for (int i = 1; i <= 11; ++i)
		disableMenuItem(_actionMenu[i]._menuId, _actionMenu[i]._actionId);

	setText(OPCODE_SOUND >> 8, OPCODE_SOUND & 0xFF, _vm->getEngineString(S_SUITE));
	setText(OPCODE_LIFT  >> 8, OPCODE_LIFT  & 0xFF, _vm->getEngineString(S_STOP));
}

/**
 * Engine function - Switch action menu from "Search" mode back to normal mode
 * @remarks	Originally called 'mfouen'
 */
void Menu::unsetSearchMenu() {
	setDestinationText(_vm->_coreVar._currPlace);
	for (int i = 1; i <= 11; ++i)
		enableMenuItem(_actionMenu[i]._menuId, _actionMenu[i]._actionId);

	setText(OPCODE_SOUND >> 8, OPCODE_SOUND & 0xFF, _vm->getEngineString(S_PROBE));
	setText(OPCODE_LIFT  >> 8, OPCODE_LIFT  & 0xFF, _vm->getEngineString(S_RAISE));
}

/**
 * Set Inventory menu texts
 * @remarks	Originally called 'modinv'
 */
void Menu::setInventoryText() {
	Common::String nomp;

	int cy = 0;
	for (int i = 1; i <= 6; ++i) {
		if (_vm->_coreVar._inventory[i] != 0) {
			++cy;
			int r = _vm->_coreVar._inventory[i] + 400;
			nomp = _vm->getString(r - 501 + kInventoryStringIndex);
			setText(_inventoryMenu[cy]._menuId, _inventoryMenu[cy]._actionId, nomp);
			enableMenuItem(_inventoryMenu[i]._menuId, _inventoryMenu[i]._actionId);
		}
	}

	if (cy < 6) {
		for (int i = cy + 1; i <= 6; ++i) {
			setText(_inventoryMenu[i]._menuId, _inventoryMenu[i]._actionId, "                       ");
			disableMenuItem(_inventoryMenu[i]._menuId, _inventoryMenu[i]._actionId);
		}
	}
}
} // End of namespace Mortevielle
