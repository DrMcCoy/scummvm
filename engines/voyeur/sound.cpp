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

#include "audio/audiostream.h"
#include "audio/decoders/raw.h"
#include "common/memstream.h"
#include "voyeur/sound.h"
#include "voyeur/staticres.h"

namespace Voyeur {

SoundManager::SoundManager(Audio::Mixer *mixer) {
	_mixer = mixer;
}

void SoundManager::playVOCMap(byte *voc, int vocSize) {
	Common::MemoryReadStream *dataStream = new Common::MemoryReadStream(voc, vocSize, DisposeAfterUse::NO);
	Audio::AudioStream *audioStream = Audio::makeVOCStream(dataStream, Audio::FLAG_UNSIGNED, DisposeAfterUse::YES);

	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundHandle, audioStream);
}

bool SoundManager::vocMapStatus() {
	// TODO
	return false;
}

void SoundManager::continueVocMap() {
	// No implementation needed in ScummVM
}

void SoundManager::abortVOCMap() {
	_mixer->stopHandle(_soundHandle);
}

void SoundManager::stopVOCPlay() {
	_mixer->stopHandle(_soundHandle);
}

void SoundManager::setVOCOffset(int offset) {
	error("TODO: setVOCOffset");
}

Common::String SoundManager::getVOCFileName(int idx) {
	return Common::String::format("%s.voc", VOC_FILENAMES[idx]);
}

void SoundManager::startVOCPlay(const Common::String &filename) {
	Common::File f;
	if (!f.open(filename))
		error("Could not find voc file - %s", filename.c_str());

	Audio::AudioStream *audioStream = Audio::makeVOCStream(f.readStream(f.size()), 
		Audio::FLAG_UNSIGNED, DisposeAfterUse::YES);

	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundHandle, audioStream);
}

int SoundManager::getVOCStatus() {
	return _mixer->isSoundHandleActive(_soundHandle);
}

} // End of namespace Voyeur
