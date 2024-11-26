#include "platform.h"

#if defined(_WIN32) || defined(_WIN64)
#define NOMINMAX
#include <Windows.h>

extern "C" void reset() { ConsoleInput::reset(); }

DWORD mode;
HANDLE stdinHandle = INVALID_HANDLE_VALUE;
HANDLE stdoutHandle = INVALID_HANDLE_VALUE;

bool ConsoleInput::isRaw = false;

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
        throw std::runtime_error("ConsoleInput::setRawMode: GetStdHandle");
    }

    if (!GetConsoleMode(stdinHandle, &mode)) {
        throw std::runtime_error("ConsoleInput::setRawMode: GetConsoleMode");
    }

    DWORD raw = mode;

    raw &= ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT | ENABLE_WRAP_AT_EOL_OUTPUT);
    raw &= (ENABLE_VIRTUAL_TERMINAL_PROCESSING | DISABLE_NEWLINE_AUTO_RETURN);

    if (!SetConsoleMode(stdinHandle, mode)) {
        throw std::runtime_error("ConsoleInput::setRawMode: SetConsoleMode");
    }
    
    isRaw = true;
}

// Only use after setting raw mode
void ConsoleInput::resetCanonicalMode() {

    if (!isRaw) return;

    stdinHandle = GetStdHandle(STD_INPUT_HANDLE);
    if (stdinHandle == INVALID_HANDLE_VALUE) {
        throw std::runtime_error("ConsoleInput::resetCanonicalMode: GetStdHandle");
    }

    if (!SetConsoleMode(stdinHandle, mode)) {
        throw std::runtime_error("ConsoleInput::resetCanonicalMode: SetConsoleMode");
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

// Windows does not support ANSI by default everywhere!!!!
// Using FillConsoleOutputCharacter is not recommended, but since
// some terminals are not xterm compatible,
// this is the way to do.

// If all Windows terminals were xterm compatible,
// It'd be as easy as typing:
/*
    WriteConsole(stdoutHandle, "\x1b[2J", 4);
    SetConsoleCursorPosition(stdinHandle, coordScreen);
*/

void ConsoleInput::writeToStdout(const char* msg, int bytes) {
    if (stdoutHandle == INVALID_HANDLE_VALUE) {
        stdoutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
        if (stdoutHandle == INVALID_HANDLE_VALUE) {
            throw std::runtime_error("ConsoleInput::writeToStdout: GetStdHandle");
        }
    }
    WriteConsoleA(stdoutHandle, msg, bytes, nullptr, nullptr);
}
void ConsoleInput::clearScreen() {

    COORD coordScreen = { 0, 0 };
    DWORD dwCount;

    if (stdoutHandle == INVALID_HANDLE_VALUE) {
        stdoutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
        if (stdoutHandle == INVALID_HANDLE_VALUE) {
            throw std::runtime_error("ConsoleInput::clearScreen: GetStdHandle");
        }
    }

    // Indeed.
    if (FillConsoleOutputCharacter(stdoutHandle, ' ', dwCount, coordScreen, &dwCount) == 0) {
        throw new std::runtime_error("ConsoleInput::clearScreen: FillConsoleOutputCharacter");
    }

    if (SetConsoleCursorPosition(stdoutHandle, coordScreen) == 0 ) {
        throw new std::runtime_error("ConsoleInput::clearScreen: SetConsoleCursorPosition");
    }
}

void ConsoleInput::resetCursor() {
    
    if (stdoutHandle == INVALID_HANDLE_VALUE) {
        stdoutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
        if (stdoutHandle == INVALID_HANDLE_VALUE) {
            throw std::runtime_error("ConsoleInput::resetCursor: GetStdHandle");
        }
    }

    COORD coordScreen = { 0, 0 };
    if (SetConsoleCursorPosition(stdoutHandle, coordScreen) == 0 ) {
        throw new std::runtime_error("ConsoleInput::resetCursor: SetConsoleCursorPosition");
    }
}

bool ConsoleInput::isInitialized() { return isRaw; }

#endif