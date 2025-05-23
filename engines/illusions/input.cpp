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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "illusions/input.h"
#include "common/system.h"

namespace Illusions {

// KeyMap

void KeyMap::addKey(Common::CustomEventType action) {
	add(action, MOUSE_NONE);
}

void KeyMap::addMouseButton(int mouseButton) {
	add(kActionNone, mouseButton);
}

void KeyMap::add(Common::CustomEventType action, int mouseButton) {
	KeyMapping keyMapping;
	keyMapping._action = action;
	keyMapping._mouseButton = mouseButton;
	keyMapping._down = false;
	push_back(keyMapping);
}

// InputEvent

InputEvent::InputEvent() : _bitMask(0) {
}

InputEvent& InputEvent::setBitMask(uint bitMask) {
	_bitMask = bitMask;
	return *this;
}

InputEvent& InputEvent::addKey(Common::CustomEventType action) {
	_keyMap.addKey(action);
	return *this;
}

InputEvent& InputEvent::addMouseButton(int mouseButton) {
	_keyMap.addMouseButton(mouseButton);
	return *this;
}

uint InputEvent::handle(Common::CustomEventType action, int mouseButton, bool down) {
	uint newKeys = 0;
	for (auto &keyMapping : _keyMap) {
		if ((keyMapping._action != kActionNone && keyMapping._action == action) ||
			(keyMapping._mouseButton != MOUSE_NONE && keyMapping._mouseButton == mouseButton)) {
			if (down && !keyMapping._down) {
				newKeys |= _bitMask;
				keyMapping._down = true;
			} else if (!down)
				keyMapping._down = false;
		}
	}
	return newKeys;
}

// Input

const uint kAllButtons = 0xFFFF;
static const char kCheatCode[] = "gosanta";

Input::Input() {
	_buttonStates = 0;
	_newButtons = 0;
	_buttonsDown = 0;
	_newKeys = 0;
	_enabledButtons = kAllButtons;
	_cursorPos.x = 0;
	_cursorPos.y = 0;
	_prevCursorPos.x = 0;
	_prevCursorPos.y = 0;
	_cursorMovedByKeyboard = false;
	_cheatCodeIndex = 0;
}

void Input::processEvent(Common::Event event) {
	switch (event.type) {
	case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
		handleAction(event.customType, MOUSE_NONE, true);
		break;
	case Common::EVENT_CUSTOM_ENGINE_ACTION_END:
		handleAction(event.customType, MOUSE_NONE, false);
		break;
	case Common::EVENT_KEYDOWN:
		handleKey(event.kbd.keycode, MOUSE_NONE, true);
		break;
	case Common::EVENT_KEYUP:
		handleKey(event.kbd.keycode, MOUSE_NONE, false);
		break;
	case Common::EVENT_MOUSEMOVE:
		_cursorMovedByKeyboard = false;
		_cursorPos.x = event.mouse.x;
		_cursorPos.y = event.mouse.y;
		break;
	case Common::EVENT_LBUTTONDOWN:
		handleMouseButton(MOUSE_LEFT_BUTTON, true);
		break;
	case Common::EVENT_LBUTTONUP:
		handleMouseButton(MOUSE_LEFT_BUTTON, false);
		break;
	case Common::EVENT_RBUTTONDOWN:
		handleMouseButton(MOUSE_RIGHT_BUTTON, true);
		break;
	case Common::EVENT_RBUTTONUP:
		handleMouseButton(MOUSE_RIGHT_BUTTON, false);
		break;
	default:
		break;
	}
}

bool Input::pollEvent(uint evt) {
	return pollButton(_inputEvents[evt].getBitMask());
}

bool Input::hasNewEvents() {
	return lookNewButtons(kAllButtons);
}

void Input::discardEvent(uint evt) {
	discardButtons(_inputEvents[evt].getBitMask());
}

void Input::discardAllEvents() {
	discardButtons(kAllButtons);
}

void Input::activateButton(uint bitMask) {
	_enabledButtons |= bitMask;
	_buttonStates &= ~bitMask;
}

void Input::deactivateButton(uint bitMask) {
	_enabledButtons &= ~bitMask;
}

Common::Point Input::getCursorPosition() {
	return _cursorPos;
}

void Input::setCursorPosition(Common::Point mousePos) {
	_prevCursorPos = _cursorPos = mousePos;
}

Common::Point Input::getCursorDelta() {
	Common::Point deltaPos;
	deltaPos.x = _prevCursorPos.x - _cursorPos.x;
	deltaPos.y = _prevCursorPos.y - _cursorPos.y;
	_prevCursorPos = _cursorPos;
	return deltaPos;
}

InputEvent& Input::setInputEvent(uint evt, uint bitMask) {
	InputEvent& inputEvent = _inputEvents[evt];
	return inputEvent.setBitMask(bitMask);
}

void Input::handleAction(Common::CustomEventType action, int mouseButton, bool down) {
	switch (action) {
	case kActionCursorUp:
		moveCursorByKeyboard(0, -4);
		break;
	case kActionCursorDown:
		moveCursorByKeyboard(0, 4);
		break;
	case kActionCursorRight:
		moveCursorByKeyboard(4, 0);
		break;
	case kActionCursorLeft:
		moveCursorByKeyboard(-4, 0);
		break;
	default:
		break;
	}
	for (uint i = 0; i < kEventMax; ++i) {
		_newKeys |= _inputEvents[i].handle(action, mouseButton, down);
	}
	uint prevButtonStates = _buttonStates;
	_buttonStates |= _newKeys;
	_newKeys = 0;
	_newButtons = ~prevButtonStates & _buttonStates;
}

void Input::handleKey(Common::KeyCode key, int mouseButton, bool down) {
	if (!down && !isCheatModeActive()) {
		if (_cheatCodeIndex < 7 && key == kCheatCode[_cheatCodeIndex]) {
			_cheatCodeIndex++;
		} else {
			_cheatCodeIndex = 0;
		}
	}
}

void Input::handleMouseButton(int mouseButton, bool down) {
	if (down)
		_buttonsDown |= mouseButton;
	else
		_buttonsDown &= ~mouseButton;
	handleAction(kActionNone, mouseButton, down);
}

bool Input::pollButton(uint bitMask) {
	if (lookButtonStates(bitMask)) {
		_buttonStates &= ~bitMask;
		return true;
	}
	return false;
}

bool Input::lookButtonStates(uint bitMask) {
	return (bitMask & (_buttonStates & _enabledButtons)) != 0;
}

bool Input::lookNewButtons(uint bitMask) {
	return (bitMask & (_newButtons & _enabledButtons)) != 0;
}

void Input::setButtonState(uint bitMask) {
	_buttonStates |= _enabledButtons & bitMask;
}

void Input::discardButtons(uint bitMask) {
	_buttonStates &= ~bitMask;
}

void Input::moveCursorByKeyboard(int deltaX, int deltaY) {
	_cursorMovedByKeyboard = true;
	_cursorPos.x = CLIP(_cursorPos.x + deltaX, 0, g_system->getWidth() - 1);
	_cursorPos.y = CLIP(_cursorPos.y + deltaY, 0, g_system->getHeight() - 1);
}

bool Input::isCheatModeActive() {
	return _cheatCodeIndex == 7;
}

void Input::setCheatModeActive(bool active) {
	_cheatCodeIndex = active ? 7 : 0;
}

} // End of namespace Illusions
