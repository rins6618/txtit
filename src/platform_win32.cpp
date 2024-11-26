#include "platform.h"

#if defined(_WIN32) || defined(_WIN64)
#define NOMINMAX
#include <Windows.h>

extern "C" void reset() { ConsoleInput::reset(); }

DWORD mode;
HANDLE stdinHandle;


struct KeyEvent {
    bool printable;
    char character;
    WORD vKeyCode;
    WORD scanCode;
};


std::optional<KeyEvent> getWinKey() {
    INPUT_RECORD ir;
    DWORD event;

    while(true) {
        if (!ReadConsoleInput(stdinHandle, &ir, 1, &event)) {
            throw new std::runtime_error("Failed to read stdin");
        }

        if (ir.EventType == KEY_EVENT) {
                const auto& keyEvent = ir.Event.KeyEvent;
                if (keyEvent.bKeyDown) {
                    return KeyEvent{
                        keyEvent.uChar.AsciiChar != 0,
                        keyEvent.uChar.AsciiChar,
                        keyEvent.wVirtualKeyCode,
                        keyEvent.wVirtualScanCode
                    };
                }
        }
    }
}

ConsoleInput::ConsoleInput() {
    ConsoleInput::setRawMode();
    atexit(reset);
}

ConsoleInput::~ConsoleInput() {
    ConsoleInput::reset();
}

void ConsoleInput::reset() { ConsoleInput::resetCanonicalMode(); }

void ConsoleInput::setRawMode() {
    
    stdinHandle = GetStdHandle(STD_INPUT_HANDLE);
    if (stdinHandle == INVALID_HANDLE_VALUE) {
        throw std::runtime_error("Invalid Handle Value");
    }

    if (!GetConsoleMode(stdinHandle, &mode)) {
        throw std::runtime_error("Console Mode Unknown");
    }

    DWORD raw = mode;

    raw &= ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT);

    if (!SetConsoleMode(stdinHandle, mode)) {
        throw std::runtime_error("Console Not Set");
    }
    
    isRaw = true;
}

// Only use after setting raw mode
void ConsoleInput::resetCanonicalMode() {

    if (!isRaw) return;

    stdinHandle = GetStdHandle(STD_INPUT_HANDLE);
    if (stdinHandle == INVALID_HANDLE_VALUE) {
        throw std::runtime_error("Invalid Handle Value");
    }

    if (!SetConsoleMode(stdinHandle, mode)) {
        throw std::runtime_error("Console Not Reset :( ");
    }

    isRaw = false;

}

char ConsoleInput::get() {
    return getWinKey()->character;
}

char ConsoleInput::get(char& out) {
    out = getWinKey()->character;
    return out;
}

#endif