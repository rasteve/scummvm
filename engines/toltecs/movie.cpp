/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "audio/decoders/raw.h"

#include "toltecs/toltecs.h"
#include "toltecs/movie.h"
#include "toltecs/palette.h"
#include "toltecs/resource.h"
#include "toltecs/screen.h"
#include "toltecs/script.h"

namespace Toltecs {

enum ChunkTypes {
	kChunkFirstImage = 0,
	kChunkSubsequentImages = 1,
	kChunkPalette = 2,
	kChunkUnused = 3,
	kChunkAudio = 4,
	kChunkShowSubtitle = 5,
	kChunkShakeScreen = 6,
	kChunkSetupSubtitles = 7,
	kChunkStopSubtitles = 8
};

MoviePlayer::MoviePlayer(ToltecsEngine *vm) : _vm(vm), _isPlaying(false), _lastPrefetchOfs(0), _framesPerSoundChunk(0), _endPos(0), _audioStream(0) {
}

MoviePlayer::~MoviePlayer() {
}

void MoviePlayer::playMovie(uint resIndex) {
	const uint32 subtitleSlot = kMaxScriptSlots - 1;
	int16 savedSceneWidth = _vm->_sceneWidth;
	int16 savedSceneHeight = _vm->_sceneHeight;
	int16 savedCameraHeight = _vm->_cameraHeight;
	int16 savedCameraX = _vm->_cameraX;
	int16 savedCameraY = _vm->_cameraY;
	int16 savedGuiHeight = _vm->_guiHeight;
	byte moviePalette[768];

	_isPlaying = true;
	_vm->_isSaveAllowed = false;

	memset(moviePalette, 0, sizeof(moviePalette));

	_vm->_screen->finishTalkTextItems();

	_vm->_arc->openResource(resIndex);
	_endPos = _vm->_arc->pos() + _vm->_arc->getResourceSize(resIndex);

	/*_frameCount = */_vm->_arc->readUint32LE();
	uint32 chunkCount = _vm->_arc->readUint32LE();

	// TODO: Figure out rest of the header
	_vm->_arc->readUint32LE();
	_vm->_arc->readUint32LE();
	_framesPerSoundChunk = _vm->_arc->readUint32LE();
	int rate = _vm->_arc->readUint32LE();

	_vm->_sceneWidth = 640;
	_vm->_sceneHeight = 400;
	_vm->_cameraHeight = 400;
	_vm->_cameraX = 0;
	_vm->_cameraY = 0;
	_vm->_guiHeight = 0;

	_audioStream = Audio::makeQueuingAudioStream(rate, false);

	_vm->_mixer->playStream(Audio::Mixer::kPlainSoundType, &_audioStreamHandle, _audioStream);

	_lastPrefetchOfs = 0;

	fetchAudioChunks();

	byte *chunkBuffer = NULL;
	uint32 chunkBufferSize = 0;
	uint32 frame = 0;
	bool abortMovie = false;
	uint32 soundChunkFramesLeft = 0;

	while (chunkCount-- && !abortMovie) {
		byte chunkType = _vm->_arc->readByte();
		uint32 chunkSize = _vm->_arc->readUint32LE();

		debug(0, "chunkType = %d; chunkSize = %d", chunkType, chunkSize);

		// Skip audio chunks - we've already queued them in
		// fetchAudioChunks()
		if (chunkType == kChunkAudio) {
			_vm->_arc->skip(chunkSize);
			soundChunkFramesLeft += _framesPerSoundChunk;
		} else {
			// Only reallocate the chunk buffer if the new chunk is bigger
			if (chunkSize > chunkBufferSize) {
				delete[] chunkBuffer;
				chunkBuffer = new byte[chunkSize];
				chunkBufferSize = chunkSize;
			}

			_vm->_arc->read(chunkBuffer, chunkSize);
		}

		switch (chunkType) {
		case kChunkFirstImage:
		case kChunkSubsequentImages:
			unpackRle(chunkBuffer, _vm->_screen->_backScreen);
			_vm->_screen->_fullRefresh = true;

			if (--soundChunkFramesLeft <= _framesPerSoundChunk) {
				fetchAudioChunks();
			}

			while (_vm->_mixer->getSoundElapsedTime(_audioStreamHandle) < (1000 * frame) / 9) {
				if (_vm->_screen->_shakeActive && _vm->_screen->updateShakeScreen()) {
					_vm->_screen->_fullRefresh = true;
				}
				if (!handleInput())
					abortMovie = true;
				_vm->drawScreen();
				// Note: drawScreen() calls delayMillis()
			}

			frame++;
			break;
		case kChunkPalette:
			unpackPalette(chunkBuffer, moviePalette, 256, 3);
			_vm->_palette->setFullPalette(moviePalette);
			break;
		case kChunkUnused:
			error("Chunk considered to be unused has been encountered");
		case kChunkAudio:
			// Already processed
			break;
		case kChunkShowSubtitle:
			memcpy(_vm->_script->getSlotData(subtitleSlot), chunkBuffer, chunkSize);
			// The last character of the subtitle determines if it should
			// always be displayed or not. If it's 0xFF, it should always be
			// displayed, otherwise, if it's 0xFE, it can be toggled.
			_vm->_screen->updateTalkText(subtitleSlot, 0, (chunkBuffer[chunkSize - 1] == 0xFF));
			break;
		case kChunkShakeScreen: // start/stop shakescreen effect
			if (chunkBuffer[0] == 0xFF)
				_vm->_screen->stopShakeScreen();
			else
				_vm->_screen->startShakeScreen(chunkBuffer[0]);
			break;
		case kChunkSetupSubtitles: // setup subtitle parameters
			_vm->_screen->_talkTextY = READ_LE_UINT16(chunkBuffer + 0);
			_vm->_screen->_talkTextX = READ_LE_UINT16(chunkBuffer + 2);
			_vm->_screen->_talkTextFontColor = ((chunkBuffer[4] << 4) & 0xF0) | ((chunkBuffer[4] >> 4) & 0x0F);
			debug(0, "_talkTextX = %d; _talkTextY = %d; _talkTextFontColor = %d",
				_vm->_screen->_talkTextX, _vm->_screen->_talkTextY, _vm->_screen->_talkTextFontColor);
			break;
		case kChunkStopSubtitles:
			_vm->_script->getSlotData(subtitleSlot)[0] = 0xFF;
			_vm->_screen->finishTalkTextItems();
			break;
		default:
			error("MoviePlayer::playMovie(%04X) Unknown chunk type %d at %08X", resIndex, chunkType, (int)_vm->_arc->pos() - 5 - chunkSize);
		}

		if (!handleInput())
			abortMovie = true;
	}

	delete[] chunkBuffer;

	_audioStream->finish();
	_vm->_mixer->stopHandle(_audioStreamHandle);

	_vm->_arc->closeResource();

	debug(0, "playMovie() done");

	_vm->_sceneWidth = savedSceneWidth;
	_vm->_sceneHeight = savedSceneHeight;
	_vm->_cameraHeight = savedCameraHeight;
	_vm->_cameraX = savedCameraX;
	_vm->_cameraY = savedCameraY;
	_vm->_guiHeight = savedGuiHeight;

	_vm->_isSaveAllowed = true;
	_isPlaying = false;
}

void MoviePlayer::fetchAudioChunks() {
	uint32 startOfs = _vm->_arc->pos();
	uint prefetchChunkCount = 0;

	if (_lastPrefetchOfs != 0)
		_vm->_arc->seek(_lastPrefetchOfs, SEEK_SET);

	while (prefetchChunkCount < _framesPerSoundChunk / 2 && _vm->_arc->pos() < _endPos) {
		byte chunkType = _vm->_arc->readByte();
		uint32 chunkSize = _vm->_arc->readUint32LE();
		if (chunkType == kChunkAudio) {
			byte *chunkBuffer = (byte *)malloc(chunkSize);
			_vm->_arc->read(chunkBuffer, chunkSize);
			_audioStream->queueBuffer(chunkBuffer, chunkSize, DisposeAfterUse::YES, Audio::FLAG_UNSIGNED);
			chunkBuffer = NULL;
			prefetchChunkCount++;
		} else {
			_vm->_arc->seek(chunkSize, SEEK_CUR);
		}
	}

	_lastPrefetchOfs = _vm->_arc->pos();

	_vm->_arc->seek(startOfs, SEEK_SET);
}

void MoviePlayer::unpackPalette(byte *source, byte *dest, int elemCount, int elemSize) {
	int ofs = 0, size = elemCount * elemSize;
	while (ofs < size) {
		byte len;
		len = *source++;
		if (len == 0) {
			len = *source++;
		} else {
			byte value = *source++;
			memset(dest, value, len);
		}
		ofs += len;
		dest += len;
	}
}

void MoviePlayer::unpackRle(byte *source, byte *dest) {
	int size = 256000;	// 640x400
	//int packedSize = 0;
	while (size > 0) {
		byte a = *source++;
		byte b = *source++;
		//packedSize += 2;
		if (a == 0) {
			dest += b;
			size -= b;
		} else {
			memset(dest, b, a);
			dest += a;
			size -= a;
		}
	}
	//debug("Packed RLE size: %d", packedSize);
}

bool MoviePlayer::handleInput() {
	Common::Event event;
	Common::EventManager *eventMan = g_system->getEventManager();
	while (eventMan->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
			switch (event.customType) {
			case kActionSkipMovie:
				return false;
			case kActionMenuOpen:
				// TODO: The original would bring up a stripped down
				// main menu dialog, without the save/restore options.
				break;
			default:
				break;
			}
			break;
		case Common::EVENT_LBUTTONDOWN:
		case Common::EVENT_RBUTTONDOWN:
			return false;
		case Common::EVENT_QUIT:
			return false;
		default:
			break;
		}
	}
	return !_vm->shouldQuit();
}

} // End of namespace Toltecs
