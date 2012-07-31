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

#include "base/plugins.h"
#include "engines/advancedDetector.h"

#include "darkseed2/darkseed2.h"
#include "darkseed2/saveload.h"
#include "graphics/surface.h"

namespace DarkSeed2 {

const char *DarkSeed2Engine::getGameId() const {
	return _gameDescription->gameid;
}

Common::Language DarkSeed2Engine::getLanguage() const {
	return _gameDescription->language;
}

Common::Platform DarkSeed2Engine::getPlatform() const {
	return _gameDescription->platform;
}

bool DarkSeed2Engine::isDemo() const {
	return (_gameDescription->flags & ADGF_DEMO) != 0;
}

} // End of namespace DarkSeed2

using namespace Common;

static const PlainGameDescriptor darkseed2Games[] = {
	{"darkseed2", "Dark Seed II"},
	{0, 0}
};

namespace DarkSeed2 {

static const ADGameDescription gameDescriptions[] = {
	{ // English version
		"darkseed2",
		"",
		{
			{"gfile.hdr",    0, "a7092ebd44d4bd70acd6dd14e111576f", 211230},
			{"gl00_txt.000", 0, "0f1c8f78fa670e015115b9f2dcdcd4ae", 125377},
		},
		EN_ANY,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},
	{ // German version from the PC Joker. English speech, German text
		"darkseed2",
		"",
		{
			{"gfile.hdr",    0, "454ab83dfb35a7232ee0eb635427f761", 210856},
			{"gl00_txt.000", 0, "e195b792c29e53717a6364b66721731f", 140771},
		},
		DE_DEU,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},
	{ // French version. English speech, French text
		"darkseed2",
		"",
		{
			{"gfile.hdr",    0, "454ab83dfb35a7232ee0eb635427f761", 210856},
			{"gl00_txt.000", 0, "edbd13f748c306a4e61eb4ca2f41d3d8", 139687},
		},
		FR_FRA,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},
	{ // English Demo version
		"darkseed2",
		"Demo",
		{
			{"gl00_txt.000", 0, "3353aaa3cf28be529a2979cc78d89697", 57343},
		},
		EN_ANY,
		kPlatformWindows,
		ADGF_DEMO,
		GUIO0()
	},
	{ // Japanese Sega Saturn version
		"darkseed2",
		"",
		{
			{"initial.idx",  0, "92438db5d1b4b8946ec59ecfbd6107c0", 1284},
			{"initial.glu",  0, "558a62491c612a890a25991016ab3f81", 540300},
			{"conv0001.pgf", 0, "a2faad74e14ba51dbcf8e9fccfae1606", 10760}
		},
		JA_JPN,
		kPlatformSaturn,
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NOSPEECH)
	},
	{ // English Macintosh Version
		"darkseed2",
		"",
		{
			{"talk", 0, "1b0b8c0b264aeff60394b416afcb1e18", 963703},
			{"action", 0, "bc7a9c5900609093e37c8a5c01340ddc", 1915496}
		},
		EN_ANY,
		kPlatformMacintosh,
		ADGF_MACRESFORK,
		GUIO0()
	},
	AD_TABLE_END_MARKER
};

static const ADGameDescription fallbackDescs[] = {
	{ // 0
		"darkseed2",
		"unknown",
		AD_ENTRY1(0, 0),
		UNK_LANG,
		kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO0()
	},
	{ // 1
		"darkseed2",
		"unknown",
		AD_ENTRY1(0, 0),
		UNK_LANG,
		kPlatformSaturn,
		ADGF_NO_FLAGS,
		GUIO0()
	}
};

static const ADFileBasedFallback fileBased[] = {
	{ &fallbackDescs[ 0], { "gfile.hdr", "dark0001.exe", 0 } },
	{ &fallbackDescs[ 1], { "initial.idx", "initial.glu", "conv0000.pgf", 0 } },
	{ 0, { 0 } }
};

}

class DarkSeed2MetaEngine : public AdvancedMetaEngine {
public:
	DarkSeed2MetaEngine() : AdvancedMetaEngine(DarkSeed2::gameDescriptions, sizeof(ADGameDescription), darkseed2Games) {
		_md5Bytes = 10000;
		_singleid = "darkseed2";
		_guioptions = GUIO_NOLAUNCHLOAD;
	}

	virtual const ADGameDescription *fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist) const {
		return detectGameFilebased(allFiles, fslist, DarkSeed2::fileBased);
	}

	virtual const char *getName() const {
		return "Dark Seed II Engine";
	}

	virtual const char *getOriginalCopyright() const {
		return "Dark Seed II (C) Cyberdreams, Inc., Destiny Software Productions, Inc.";
	}

	virtual bool hasFeature(MetaEngineFeature f) const;
	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;

	SaveStateList listSaves(const char *target) const;
	int getMaximumSaveSlot() const;
	void removeSaveState(const char *target, int slot) const;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const;

};

bool DarkSeed2MetaEngine::hasFeature(MetaEngineFeature f) const {
	return (f == kSupportsListSaves           ) ||
	       (f == kSupportsLoadingDuringStartup) ||
	       (f == kSupportsDeleteSave          ) ||
	       (f == kSavesSupportMetaInfo        ) ||
	       (f == kSavesSupportThumbnail       ) ||
	       (f == kSavesSupportCreationDate    ) ||
	       (f == kSavesSupportPlayTime        );
	return false;
}

bool DarkSeed2::DarkSeed2Engine::hasFeature(EngineFeature f) const {
	return (f == kSupportsRTL                 ) ||
	       (f == kSupportsLoadingDuringRuntime) ||
	       (f == kSupportsSavingDuringRuntime ) ||
	       (f == kSupportsSubtitleOptions     );
}

SaveStateList DarkSeed2MetaEngine::listSaves(const char *target) const {
	SaveStateList list;
	if (!DarkSeed2::SaveLoad::getStates(list, target))
		return SaveStateList();

	return list;
}

int DarkSeed2MetaEngine::getMaximumSaveSlot() const {
	return DarkSeed2::SaveLoad::kMaxSlot;
}

void DarkSeed2MetaEngine::removeSaveState(const char *target, int slot) const {
	DarkSeed2::SaveLoad::removeSave(target, slot);
}

SaveStateDescriptor DarkSeed2MetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	SaveStateDescriptor state;
	if (!DarkSeed2::SaveLoad::getState(state, target, slot))
		return SaveStateDescriptor();

	return state;
}

bool DarkSeed2MetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	if (desc) {
		*engine = new DarkSeed2::DarkSeed2Engine(syst, desc);
		((DarkSeed2::DarkSeed2Engine *)*engine)->initGame(desc);
	}
	return desc != 0;
}

#if PLUGIN_ENABLED_DYNAMIC(DARKSEED2)
	REGISTER_PLUGIN_DYNAMIC(DARKSEED2, PLUGIN_TYPE_ENGINE, DarkSeed2MetaEngine);
#else
	REGISTER_PLUGIN_STATIC(DARKSEED2, PLUGIN_TYPE_ENGINE, DarkSeed2MetaEngine);
#endif

namespace DarkSeed2 {

void DarkSeed2Engine::initGame(const ADGameDescription *gd) {
}

} // End of namespace DarkSeed2
