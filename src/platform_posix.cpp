#include "platform.h"
#include "txtit.h"
#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#include <unistd.h>
#include <termio.h>
#include <sstream>

bool resized = false;

/* Static definitions */
bool ConsoleInput::isRaw = false;

/* Private Methods*/
void ConsoleInput::updateConsoleState() {
    bool resizedCopy = resized;
    resized = false;
    winsize ws;
    Coords dimensions {};
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws ) == -1 || ws.ws_col == 0) {
        if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) {
            ERR("ConsoleInput::updateConsoleState: ioctl->write");
        }
        dimensions = getCursor();
    } else {
        dimensions = Coords { ws.ws_col, ws.ws_row };
    }
    consoleState = ConsoleState {
        dimensions.x,
        dimensions.y,
        resizedCopy
    };
}

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
        ERR("ConsoleInput::setRawMode: tcgetattr");
    }

    termios raw = original;
    raw.c_iflag &= ~(ICRNL | IXON | BRKINT | INPCK | ISTRIP);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= CS8;
    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        ERR("ConsoleInput::setRawMode: tcsetattr");
    }
    isRaw = true;
}

// Only use after setting raw mode
void ConsoleInput::resetCanonicalMode() {

    if (!isRaw) return;
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &original) == -1) {
        ERR("ConsoleInput::resetCanonicalMode: tcsetattr");
    }
    isRaw = false;

}

char ConsoleInput::get() {
    int nread;
    char out;
    while ((nread = read(STDIN_FILENO, &out, 1)) != 1) {
        if (nread == -1 && errno != EAGAIN) {
            ERR("ConsoleInput::get");
        }
    }
    return out;
}

char ConsoleInput::get(char& out) {
    int nread;
    while ((nread = read(STDIN_FILENO, &out, 1)) != 1) {
        if (nread == -1 && errno != EAGAIN) {
            ERR("ConsoleInput::get(out)");
        }
    }
    return out;
}

void ConsoleInput::writeToStdout(const char* msg, int bytes) {
    write(STDOUT_FILENO, msg, bytes);
}

void ConsoleInput::clearScreen() {
    ConsoleInput::writeToStdout("\x1b[2J", 4);
    ConsoleInput::writeToStdout("\x1b[3J", 4);
    ConsoleInput::writeToStdout("\x1b[H", 3);
}

void ConsoleInput::resetCursor() {
    writeToStdout("\x1b[H", 3);
}

void ConsoleInput::setCursor(Coords pos) {
    std::stringstream ss;
    ss << "\x1b[" << (pos.y + 1) << ';' << (pos.x + 1) << 'f';
    std::string command = ss.str(); 
    
    writeToStdout(command.c_str(), command.length());
}

Coords ConsoleInput::getCursor() {
    char buf[32];
    unsigned int i = 0;
    
    if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) ERR("ConsoleInput::getCursor: write");
    std::cout << "\r\n";
    while (i < sizeof(buf) - 1) {
        if (read(STDIN_FILENO, &buf[i], 1) != 1) break;
        if (buf[i] == 'R') break;
        i++;
        }
    buf[i] = '\0';

    Coords position {};
    if (buf[0] != '\x1b' || buf[1] != '[') ERR("Escape Sequence Expected");
    if (sscanf(&buf[2], "%d;%d", &position.y, &position.x) != 2) ERR("Cursor Position not found");
    return position;
}

ConsoleInput::ConsoleState ConsoleInput::getConsoleState() {
    updateConsoleState();
    return consoleState;
}

bool ConsoleInput::isInitialized() { return isRaw; }


#endif