#include "platform.h"

#if defined(_WIN32) || defined(_WIN64)
/* Static definitions */
bool ConsoleInput::isRaw = false;

/* Windows Specific */
#define NOMINMAX
#include <Windows.h>
void reset() { ConsoleInput::reset(); }

DWORD stdinMode;
DWORD stdoutMode;
HANDLE stdinHandle = INVALID_HANDLE_VALUE;
HANDLE stdoutHandle = INVALID_HANDLE_VALUE;

struct KeyEvent {
    bool printable;
    char character;
    WORD vKeyCode;
    WORD scanCode;
};

bool resized = false;

std::optional<KeyEvent> getWinKey() {
    INPUT_RECORD ir;
    DWORD event;

    while(true) {
        if (!ReadConsoleInput(stdinHandle, &ir, 1, &event)) {
            throw new std::runtime_error("getWinKey: Failed to read console");
        }

        if (ir.EventType == WINDOW_BUFFER_SIZE_EVENT) {
            resized = true;
            return std::nullopt;
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

/* Class Implementations */
/* Private Methods*/

void ConsoleInput::updateConsoleState() {
    bool resizedCopy = resized;
    resized = false;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo( GetStdHandle( STD_OUTPUT_HANDLE ), &csbi);
    
    COORD newSize = {
        csbi.srWindow.Right - csbi.srWindow.Left + 1, // Window width
        csbi.srWindow.Bottom - csbi.srWindow.Top + 1 // Window height
    };
    
    consoleState = ConsoleState {
        newSize.X,
        newSize.Y,
        resizedCopy
    };
}

/* Static Private Methods*/

void ConsoleInput::setRawMode() {
    
    if (stdinHandle == INVALID_HANDLE_VALUE) {
        stdinHandle = GetStdHandle(STD_INPUT_HANDLE);
        if (stdinHandle == INVALID_HANDLE_VALUE) {
            throw std::runtime_error("ConsoleInput::setRawMode: GetStdHandle (stdin)");
        }
    }

    if (stdoutHandle == INVALID_HANDLE_VALUE) {
        stdoutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
        if (stdoutHandle == INVALID_HANDLE_VALUE) {
            throw std::runtime_error("ConsoleInput::setRawMode: GetStdHandle (stdout)");
        }
    }

    if (!GetConsoleMode(stdinHandle, &stdinMode)) {
        throw std::runtime_error("ConsoleInput::setRawMode: GetConsoleMode (stdin)");
    }

    if (!GetConsoleMode(stdoutHandle, &stdoutMode)) {
        throw std::runtime_error("ConsoleInput::setRawMode: GetConsoleMode (stdout)");
    }

    DWORD rawIn = stdinMode;
    DWORD rawOut = stdoutMode;

    rawIn &= ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT);
    rawIn |= ENABLE_WINDOW_INPUT;
    rawOut &= (ENABLE_PROCESSED_OUTPUT | DISABLE_NEWLINE_AUTO_RETURN);

    if (!SetConsoleMode(stdinHandle, rawIn)) {
        throw std::runtime_error("ConsoleInput::setRawMode: SetConsoleMode (stdin)");
    }

    if (!SetConsoleMode(stdoutHandle, rawOut)) {
        throw std::runtime_error("ConsoleInput::setRawMode: SetConsoleMode (stdout)");
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

    if (!SetConsoleMode(stdinHandle, stdinMode)) {
        throw std::runtime_error("ConsoleInput::resetCanonicalMode: SetConsoleMode");
    }

    isRaw = false;

}


/* Public Methods */
ConsoleInput::ConsoleInput() {
    // idk how to clear history buffer so this works
    ConsoleInput::setRawMode();
    atexit(reset);
    updateConsoleState();
}

ConsoleInput::~ConsoleInput() {
    ConsoleInput::reset();
}



char ConsoleInput::get() {
    auto c = getWinKey();
    if (!c.has_value()) return '\0';
    return c->character;
}

char ConsoleInput::get(char& out) {
    auto c = getWinKey();
    if (!c.has_value()) {
        out = '\0';
    } else {
        out = c->character;
    }
    return out;
}

void ConsoleInput::writeToStdout(const char* msg, int bytes) {
    if (stdoutHandle == INVALID_HANDLE_VALUE) {
        stdoutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
        if (stdoutHandle == INVALID_HANDLE_VALUE) {
            throw std::runtime_error("ConsoleInput::writeToStdout: GetStdHandle");
        }
    }
    WriteConsoleA(stdoutHandle, msg, bytes, nullptr, nullptr);
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

void ConsoleInput::clearScreen() {
    
    COORD coordScreen = { 0, 0 };
    DWORD dwCount;

    if (stdoutHandle == INVALID_HANDLE_VALUE) {
        stdoutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
        if (stdoutHandle == INVALID_HANDLE_VALUE) {
            throw std::runtime_error("ConsoleInput::clearScreen: GetStdHandle");
        }
    }

    // In case the console is xterm compatible:
    writeToStdout("\x1b[2J", 4);
    writeToStdout("\x1b[3J", 4);

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

ConsoleInput::ConsoleState ConsoleInput::getConsoleState() {
    updateConsoleState();
    return consoleState;
}


/* Static Public Methods */
void ConsoleInput::reset() { ConsoleInput::resetCanonicalMode(); }
bool ConsoleInput::isInitialized() { return isRaw; }

#endif