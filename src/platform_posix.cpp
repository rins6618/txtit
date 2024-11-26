#include "platform.h"
#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#include <unistd.h>
#include <termio.h>

bool ConsoleInput::isRaw = false;

ConsoleInput::ConsoleInput() {
    ConsoleInput::setRawMode();
    atexit(reset);
}

ConsoleInput::~ConsoleInput() {
    ConsoleInput::reset();
}

void ConsoleInput::reset() { ConsoleInput::resetCanonicalMode(); }

termios original;

void ConsoleInput::setRawMode() {

    if (isRaw) return;
    if (tcgetattr(STDIN_FILENO, &original) == -1){
        throw new std::runtime_error("ConsoleInput::setRawMode: tcgetattr");
    }

    termios raw = original;
    raw.c_iflag &= ~(ICRNL | IXON | BRKINT | INPCK | ISTRIP);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= CS8;
    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        throw new std::runtime_error("ConsoleInput::setRawMode: tcsetattr");
    }
    isRaw = true;
}

// Only use after setting raw mode
void ConsoleInput::resetCanonicalMode() {

    if (!isRaw) return;
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &original) == -1) {
        throw new std::runtime_error("ConsoleInput::resetCanonicalMode: tcsetattr");
    }
    isRaw = false;

}

char ConsoleInput::get() {
    int nread;
    char out;
    while ((nread = read(STDIN_FILENO, &out, 1)) != 1) {
        if (nread == -1 && errno != EAGAIN) {
            throw new std::runtime_error("ConsoleInput::get");
        }
    }
    return out;
}

char ConsoleInput::get(char& out) {
    int nread;
    while ((nread = read(STDIN_FILENO, &out, 1)) != 1) {
        if (nread == -1 && errno != EAGAIN) {
            throw new std::runtime_error("ConsoleInput::get(out)");
        }
    }
    return out;
}

bool ConsoleInput::isInitialized() { return isRaw; }


#endif