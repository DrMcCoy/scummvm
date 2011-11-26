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

#include "dreamweb/dreamweb.h"
#include "engines/metaengine.h"
#include "gui/saveload.h"
#include "common/config-manager.h"
#include "common/translation.h"

namespace DreamGen {

void DreamGenContext::loadgame() {
	if (data.byte(kCommandtype) != 246) {
		data.byte(kCommandtype) = 246;
		commandonly(41);
	}
	if (data.word(kMousebutton) == data.word(kOldbutton))
		return; // "noload"
	if (data.word(kMousebutton) == 1) {
		ax = 0xFFFF;
		doload();
	}
}

// input: ax = savegameId
// if -1, open menu to ask for slot to load
// if >= 0, directly load from that slot
void DreamGenContext::doload() {
	int savegameId = (int16)ax;

	data.byte(kLoadingorsave) = 1;

	if (ConfMan.getBool("dreamweb_originalsaveload") && savegameId == -1) {
		showopbox();
		showloadops();
		data.byte(kCurrentslot) = 0;
		showslots();
		shownames();
		data.byte(kPointerframe) = 0;
		worktoscreenm();
		namestoold();
		data.byte(kGetback) = 0;

		while (true) {
			if (quitRequested())
				return;
			delpointer();
			readmouse();
			showpointer();
			vsync();
			dumppointer();
			dumptextline();
			RectWithCallback loadlist[] = {
				{ kOpsx+176,kOpsx+192,kOpsy+60,kOpsy+76,&DreamGenContext::getbacktoops },
				{ kOpsx+128,kOpsx+190,kOpsy+12,kOpsy+100,&DreamGenContext::actualload },
				{ kOpsx+2,kOpsx+92,kOpsy+4,kOpsy+81,&DreamGenContext::selectslot },
				{ 0,320,0,200,&DreamGenContext::blank },
				{ 0xFFFF,0,0,0,0 }
			};
			checkcoords(loadlist);
			if (data.byte(kGetback) == 1)
				break;
			if (data.byte(kGetback) == 2)
				return; // "quitloaded"
		}
	} else {

		if (savegameId == -1) {
			// Open dialog to get savegameId

			const EnginePlugin *plugin = NULL;
			Common::String gameId = ConfMan.get("gameid");
			EngineMan.findGame(gameId, &plugin);
			GUI::SaveLoadChooser *dialog = new GUI::SaveLoadChooser(_("Restore game:"), _("Restore"));
			dialog->setSaveMode(false);
			savegameId = dialog->runModalWithPluginAndTarget(plugin, ConfMan.getActiveDomainName());
			delete dialog;
		}

		if (savegameId < 0) {
			data.byte(kGetback) = 0;
			return;
		}


		// TODO: proper scheme for filename, in a separate function
		//Common::String filename = ConfMan.getActiveDomainName() + Common::String::format(".d%02d", savegameId);
		Common::String filename = Common::String::format("DREAMWEB.D%02d", savegameId);
		debug(1, "Loading from filename: %s", filename.c_str());
		engine->openSaveFileForReading(filename);

		// TODO: The below is duplicated from Loadposition
		data.word(kTimecount) = 0;
		clearchanges();

		ds = cs;
		dx = kFileheader;
		cx = kHeaderlen;
		savefileread();
		es = cs;
		di = kFiledata;
		ax = savegameId;
		if (savegameId < 7) {
			cx = 17;
			_mul(cx);
			ds = data;
			dx = kSavenames;
			_add(dx, ax);
			loadseg();
		} else {
			// For potential support of more than 7 savegame slots,
			// loading into the savenames buffer isn't always possible
			// Emulate a loadseg call:
			uint8 namebuf[17];
			engine->readFromFile(namebuf, 17);
			_add(di, 2);
		}
		ds = data; 
		dx = kStartvars;
		loadseg();
		ds = data.word(kExtras);
		dx = kExframedata;
		loadseg();
		ds = data.word(kBuffers);
		dx = kListofchanges;
		loadseg();
		ds = data;
		dx = kMadeuproomdat;
		loadseg();
		ds = cs;
		dx = kReelroutines;
		loadseg();
		closefile();
		data.byte(kGetback) = 1;
	}

	// kTempgraphics might not have been allocated if we bypassed all menus
	if (data.word(kTempgraphics) != 0xFFFF)
		getridoftemp();

	dx = data;
	es = dx;
	const Room *room = (Room *)cs.ptr(kMadeuproomdat, sizeof(Room));
	startloading(room);
	loadroomssample();
	data.byte(kRoomloaded) = 1;
	data.byte(kNewlocation) = 255;
	clearsprites();
	initman();
	initrain();
	data.word(kTextaddressx) = 13;
	data.word(kTextaddressy) = 182;
	data.byte(kTextlen) = 240;
	startup();
	worktoscreen();
	data.byte(kGetback) = 4;
}


void DreamGenContext::savegame() {
	if (data.byte(kMandead) == 2) {
		blank();
		return;
	}

	if (data.byte(kCommandtype) != 247) {
		data.byte(kCommandtype) = 247;
		commandonly(44);
	}
	if (data.word(kMousebutton) != 1)
		return;

	data.byte(kLoadingorsave) = 2;

	if (ConfMan.getBool("dreamweb_originalsaveload")) {
		showopbox();
		showsaveops();
		data.byte(kCurrentslot) = 0;
		showslots();
		shownames();
		worktoscreenm();
		namestoold();
		data.word(kBufferin) = 0;
		data.word(kBufferout) = 0;
		data.byte(kGetback) = 0;

		while (true) {
			if (quitRequested())
				return;
			delpointer();
			checkinput();
			readmouse();
			showpointer();
			vsync();
			dumppointer();
			dumptextline();

			RectWithCallback savelist[] = {
				{ kOpsx+176,kOpsx+192,kOpsy+60,kOpsy+76,&DreamGenContext::getbacktoops },
				{ kOpsx+128,kOpsx+190,kOpsy+12,kOpsy+100,&DreamGenContext::actualsave },
				{ kOpsx+2,kOpsx+92,kOpsy+4,kOpsy+81,&DreamGenContext::selectslot },
				{ 0,320,0,200,&DreamGenContext::blank },
				{ 0xFFFF,0,0,0,0 }
			};
			checkcoords(savelist);
			_cmp(data.byte(kGetback), 0);
			if (flags.z())
				continue;
			break;
		}
		return;
	} else {
		const EnginePlugin *plugin = NULL;
		Common::String gameId = ConfMan.get("gameid");
		EngineMan.findGame(gameId, &plugin);
		GUI::SaveLoadChooser *dialog = new GUI::SaveLoadChooser(_("Save game:"), _("Save"));
		dialog->setSaveMode(true);
		int savegameId = dialog->runModalWithPluginAndTarget(plugin, ConfMan.getActiveDomainName());
		Common::String game_description = dialog->getResultString();
		if (game_description.empty())
			game_description = "Untitled";
		delete dialog;

		if (savegameId < 0) {
			data.byte(kGetback) = 0;
			return;
		}

		// TODO: The below is copied from actualsave
		const Room *currentRoom = (const Room *)cs.ptr(kRoomdata + sizeof(Room)*data.byte(kLocation), sizeof(Room));
		Room *madeUpRoom = (Room *)cs.ptr(kMadeuproomdat, sizeof(Room));

		*madeUpRoom = *currentRoom;
		bx = kMadeuproomdat;
		es = cs;
		madeUpRoom->roomsSample = data.byte(kRoomssample);
		madeUpRoom->mapX = data.byte(kMapx);
		madeUpRoom->mapY = data.byte(kMapy);
		madeUpRoom->liftFlag = data.byte(kLiftflag);
		madeUpRoom->b21 = data.byte(kManspath);
		madeUpRoom->facing = data.byte(kFacing);
		madeUpRoom->b27 = 255;

		// TODO: The below is copied from saveposition
		makeheader();

		//Common::String filename = ConfMan.getActiveDomainName() + Common::String::format(".d%02d", savegameId);
		Common::String filename = Common::String::format("DREAMWEB.D%02d", savegameId);
		debug(1, "Saving to filename: %s (%s)", filename.c_str(), game_description.c_str());

		engine->openSaveFileForWriting(filename.c_str());

		dx = data;
		ds = dx;
		dx = kFileheader;
		cx = kHeaderlen;
		savefilewrite();
		dx = data;
		es = dx;
		di = kFiledata;

		// TODO: Check if this 2 is a constant
		uint8 descbuf[17] = { 2, 0 };
		strncpy((char*)descbuf+1, game_description.c_str(), 16);
		unsigned int desclen = game_description.size();
		if (desclen > 15)
			desclen = 15;
		// zero terminate, and pad with ones
		descbuf[++desclen] = 0;
		while (desclen < 17)
			descbuf[++desclen] = 1;
		if (savegameId < 7) {
			ax = savegameId;
			cx = 17;
			_mul(cx);
			ds = data;
			dx = kSavenames;
			_add(dx, ax);
			memcpy(data.ptr(dx,17), descbuf, 17);
			saveseg();
		} else {
			// savenames only has room for descriptions for 7 slots
			uint16 len = es.word(di);
			_add(di, 2);
			assert(len == 17);
			engine->writeToSaveFile(descbuf, len);
		}

		ds = data;
		dx = kStartvars;
		saveseg();
		ds = data.word(kExtras);
		dx = kExframedata;
		saveseg();
		ds = data.word(kBuffers);
		dx = kListofchanges;
		saveseg();
		ds = data;
		dx = kMadeuproomdat;
		saveseg();
		ds = data;
		dx = kReelroutines;
		saveseg();
		closefile();

		getridoftemp();
		restoreall();
		data.word(kTextaddressx) = 13;
		data.word(kTextaddressy) = 182;
		data.byte(kTextlen) = 240;
		redrawmainscrn();
		worktoscreenm();
		data.byte(kGetback) = 4;
	}
}

void DreamGenContext::namestoold() {
	memcpy(segRef(data.word(kBuffers)).ptr(kZoomspace, 0), cs.ptr(kSavenames, 0), 17*4);
}

void DreamGenContext::oldtonames() {
	memcpy(cs.ptr(kSavenames, 0), segRef(data.word(kBuffers)).ptr(kZoomspace, 0), 17*4);
}

void DreamGenContext::saveload() {
	if (data.word(kWatchingtime) || (data.byte(kPointermode) == 2)) {
		blank();
		return;
	}
	if (data.byte(kCommandtype) != 253) {
		data.byte(kCommandtype) = 253;
		commandonly(43);
	}
	if ((data.word(kMousebutton) != data.word(kOldbutton)) && (data.word(kMousebutton) & 1))
		dosaveload();
}

void DreamGenContext::showmainops() {
	showframe(tempGraphics(), kOpsx+10, kOpsy+10, 8, 0);
	showframe(tempGraphics(), kOpsx+59, kOpsy+30, 7, 0);
	showframe(tempGraphics(), kOpsx+128+4, kOpsy+12, 1, 0);
}

void DreamGenContext::showdiscops() {
	showframe(tempGraphics(), kOpsx+128+4, kOpsy+12, 1, 0);
	showframe(tempGraphics(), kOpsx+10, kOpsy+10, 9, 0);
	showframe(tempGraphics(), kOpsx+59, kOpsy+30, 10, 0);
	showframe(tempGraphics(), kOpsx+176+2, kOpsy+60-4, 5, 0);
}

void DreamGenContext::actualsave() {
	if (data.byte(kCommandtype) != 222) {
		data.byte(kCommandtype) = 222;
		commandonly(44);
	}

	if (!(data.word(kMousebutton) & 1))
		return;

	unsigned int slot = data.byte(kCurrentslot);

	const char *desc = (const char *)data.ptr(kSavenames + 17*slot + 1, 16);
	if (desc[0] == 0)
		return;

	const Room *currentRoom = (const Room *)cs.ptr(kRoomdata + sizeof(Room)*data.byte(kLocation), sizeof(Room));
	Room *madeUpRoom = (Room *)cs.ptr(kMadeuproomdat, sizeof(Room));

	*madeUpRoom = *currentRoom;
	bx = kMadeuproomdat;
	es = cs;
	madeUpRoom->roomsSample = data.byte(kRoomssample);
	madeUpRoom->mapX = data.byte(kMapx);
	madeUpRoom->mapY = data.byte(kMapy);
	madeUpRoom->liftFlag = data.byte(kLiftflag);
	madeUpRoom->b21 = data.byte(kManspath);
	madeUpRoom->facing = data.byte(kFacing);
	madeUpRoom->b27 = 255;

	saveposition();

	getridoftemp();
	restoreall(); // reels
	data.word(kTextaddressx) = 13;
	data.word(kTextaddressy) = 182;
	data.byte(kTextlen) = 240;
	redrawmainscrn();
	worktoscreenm();
	data.byte(kGetback) = 4;
}

void DreamGenContext::actualload() {
	STACK_CHECK;
	_cmp(data.byte(kCommandtype), 221);
	if (flags.z())
		goto alreadyactload;
	data.byte(kCommandtype) = 221;
	al = 41;
	commandonly();
alreadyactload:
	ax = data.word(kMousebutton);
	_cmp(ax, data.word(kOldbutton));
	if (flags.z())
		return /* (notactload) */;
	_cmp(ax, 1);
	if (!flags.z())
		return /* (notactload) */;
	dx = data;
	ds = dx;
	si = 8579;
	al = data.byte(kCurrentslot);
	ah = 0;
	cx = 17;
	_mul(cx);
	_add(si, ax);
	_inc(si);
	_cmp(ds.byte(si), 0);
	if (flags.z())
		return /* (notactload) */;
	loadposition();
	data.byte(kGetback) = 1;
}

} /*namespace dreamgen */

