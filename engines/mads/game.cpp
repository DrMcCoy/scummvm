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
#include "mads/game.h"
#include "mads/game_data.h"
#include "mads/events.h"
#include "mads/graphics.h"
#include "mads/msurface.h"
#include "mads/resources.h"
#include "mads/nebular/game_nebular.h"

namespace MADS {

Game *Game::init(MADSEngine *vm) {
	if (vm->getGameID() == GType_RexNebular)
		return new Nebular::GameNebular(vm);

	return nullptr;
}

Game::Game(MADSEngine *vm): _vm(vm), _surface(nullptr), _objects(vm), 
		_scene(vm), _player(vm) {
	_sectionNumber = _priorSectionNumber = 0;
	_difficultyLevel = DIFFICULTY_HARD;
	_saveSlot = -1;
	_statusFlag = 0;
	_sectionHandler = nullptr;
	_sectionNumber = 1;
	_priorSectionNumber = 0;
	_currentSectionNumber = -1;
	_v1 = _v2 = 0;
	_v3 = _v4 = 0;
	_v5 = _v6 = 0;
	_aaName = "*I0.AA";
	_playerSpritesFlag = false;
	_priorFrameTimer = 0;
	_updateSceneFlag = false;
	_abortTimersMode = ABORTMODE_0;
	_abortTimersMode2 = ABORTMODE_0;
	_ticksExpiry = 0;
}

Game::~Game() {
	delete _surface;
	delete _sectionHandler;
}

void Game::run() {
	initialiseGlobals();

	_statusFlag = true;
	int protectionResult = checkCopyProtection();
	switch (protectionResult) {
	case 1:
		// Copy protection failed
		_scene._nextSceneId = 804;
		_saveSlot = -1;
		break;
	case 2:
		_statusFlag = 0;
		break;
	default:
		break;
	}

	if (_saveSlot == -1 && protectionResult != -1 && protectionResult != -2) {
		initSection(_sectionNumber);
		_statusFlag = true;

		_vm->_dialogs->_pendingDialog = DIALOG_DIFFICULTY;
		_vm->_dialogs->showDialog();
		_vm->_dialogs->_pendingDialog = DIALOG_NONE;

		_priorSectionNumber = 0;
		_priorSectionNumber = -1;
		_scene._priorSceneId = 0;
		_scene._currentSceneId = -1;
	}

	if (protectionResult != 1 && protectionResult != 2) {
		initialiseGlobals();

		if (_saveSlot != -1) {
			warning("TODO: loadGame(\"REX.SAV\", 210)");
			_statusFlag = false;
		}
	}

	if (_statusFlag)
		gameLoop();
}

void Game::gameLoop() {
	while (!_vm->shouldQuit() && _statusFlag) {
		setSectionHandler();
		_sectionHandler->preLoadSection();
		initSection(_sectionNumber);
		_sectionHandler->postLoadSection();

		_scene._spriteSlots.clear(true);

		if (_sectionNumber == _currentSectionNumber) {
			sectionLoop();
		}

		// TODO: Extra reset methods
		_vm->_events->resetCursor();
		_vm->_events->freeCursors();
		_vm->_sound->closeDriver();
	}

	_vm->_palette->close();
}

void Game::sectionLoop() {
	while (!_vm->shouldQuit() && _statusFlag && _sectionNumber == _currentSectionNumber) {
		_v1 = 3;
		_player._spritesChanged = true;
		_v5 = 0;
		_v6 = 0;
		_vm->_events->resetCursor();

		_scene.clearVocab();
		_scene._dynamicHotspots.clear();
		_scene.loadSceneLogic();

		_v4 = 0;
		_player._stepEnabled = true;
		_player._visible = true;
		_vm->_dialogs->_defaultPosition = Common::Point(-1, -1);
		_visitedScenes.add(_scene._nextSceneId);

		_scene._screenObjects._v8333C = true;
		_scene._screenObjects._v832EC = 0;
		_scene._screenObjects._yp = 0;
		_v3 = -1;

		_scene._sceneLogic->setup();
		if (_player._spritesChanged || _v3) {
			if (_player._spritesLoaded)
				_scene._spriteSlots.releasePlayerSprites();
			_vm->_palette->resetGamePalette(18, 10);
			_scene._spriteSlots.clear(true);
		} else {
			_vm->_palette->initGamePalette();
		}

		_vm->_palette->_paletteUsage.load(3, 0xF0, 0xF1, 0xF2);
		
		_scene.loadScene(_scene._nextSceneId, _aaName, 0);
		_vm->_sound->pauseNewCommands();

		if (!_player._spritesLoaded) {
			_player.loadSprites("");
			_playerSpritesFlag = false;
		}

		_vm->_events->initVars();
		_scene._v1A = true;
		_scene._v1C = -1;
		_objectHiliteVocabIdx = -1;

		_scene._action.clear();
		_player.turnToDestFacing();
		_player._direction = _player._newDirection;
		_player.moveComplete();

		switch (_vm->_screenFade) {
		case SCREEN_FADE_SMOOTH:
			_abortTimers2 = 2;
			break;
		case SCREEN_FADE_FAST:
			_abortTimers2 = 20;
			break;
		default:
			_abortTimers2 = 21;
			break;
		}
		_abortTimers = 0;
		_abortTimersMode2 = ABORTMODE_1;
		_priorFrameTimer = _vm->_events->_currentTimer;

		// Call the scene logic for entering the given scene
		_scene._sceneLogic->enter();

		// Set player data
		_player._destPos = _player._playerPos;
		_player._newDirection = _player._direction;
		_player._destFacing = _player._direction;
		_player.setupFrame();
		_player.updateFrame();
		_player._visible3 = _player._visible;
		_player._special = _scene.getDepthHighBits(_player._playerPos);
		_player._priorTimer = _vm->_events->_currentTimer + _player._ticksAmount;
		_player.idle();

		warning("TODO: _selectedObject IF block");

		_v1 = 5;
		_scene._roomChanged = false;

		if ((_v5 || _v6) && !_updateSceneFlag) {
			_scene._currentSceneId = _scene._priorSceneId;
			_updateSceneFlag = true;
		} else {
			_updateSceneFlag = false;
			_scene.loop();
		}

		_vm->_events->resetCursor();
		_v1 = 3;

		delete _scene._animationData;
		_scene._animationData = nullptr;

		_scene._reloadSceneFlag = false;

		warning("TODO: sub_1DD8C, sub_1DD7E");

		if (!_playerSpritesFlag) {
			_player._spritesLoaded = false;
			_player._spritesChanged = true;
		}

		// Clear the scene
		_scene.free();
		_sectionNumber = _scene._nextSceneId / 100;

		// TODO: sub_1DD46(3)

		// Check whether to show a dialog
		checkShowDialog();
	}
}

void Game::initSection(int sectionNumber) {
	_priorSectionNumber = _currentSectionNumber;
	_currentSectionNumber = sectionNumber;

	_vm->_palette->resetGamePalette(18, 10);
	_vm->_palette->setLowRange();
	_vm->_events->loadCursors("*CURSOR.SS");
	
	assert(_vm->_events->_cursorSprites);
	_vm->_events->setCursor2((_vm->_events->_cursorSprites->getCount() <= 1) ? 
		CURSOR_ARROW : CURSOR_WAIT);
}

void Game::loadResourceSequence(const Common::String prefix, int v) {
	warning("TODO: loadResourceSequence");
}

void Game::loadQuotes() {
	File f("*QUOTES.DAT");
	int curPos = 0;

	char buffer[128];
	strcpy(buffer, "");

	while (true) {
		uint8 b = f.readByte();
		if (f.eos()) break;

		buffer[curPos++] = b;
		if (buffer[curPos - 1] == '\0') {
			// end of string, add it to the strings list
			_quotes.push_back(buffer);
			curPos = 0;
			strcpy(buffer, "");
		}
	}

	f.close();
}

} // End of namespace MADS
