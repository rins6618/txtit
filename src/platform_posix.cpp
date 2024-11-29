#include "platform.h"
#include "txtit.h"
#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#include <unistd.h>
#include <termio.h>
#include <signal.h>
#include <sstream>

bool resized = false;
Coords term_size = {0, 0};

/* POSIX specific */
Coords outerGetCursor() {
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

void resizeHandler(int sig) {
    UNUSED(sig);
    
    winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws ) == -1 || ws.ws_col == 0) {
        if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) {
            ERR("ConsoleInput::resizeHandler: ioctl->write");
        }
        term_size = outerGetCursor();
    } else {
        term_size = Coords { ws.ws_col, ws.ws_row };
    }
}

/* Static definitions */
bool ConsoleInput::isRaw = false;

/* Private Methods*/
void ConsoleInput::updateConsoleState() {
    bool resizedCopy = resized;
    resized = false;
    Coords dimensions = term_size;
    term_size = dimensions;
    consoleState = ConsoleState {
        dimensions.x,
        dimensions.y,
        resizedCopy,
        consoleState.escapeSeq
    };
}

void ConsoleInput::getEscapeSequence(char out) {
    std::string chord {out};
    bool escapeSeq = true;
    while (escapeSeq) {
        char c = get();
        chord.push_back(c);
        if (isalpha(c)) {
            escapeSeq = false;
        }
    }
    if (chord.length() < 2) return;
    if (chord.at(1) == '[') {
        switch (chord.at(2)) {
        case 'A':
            consoleState.escapeSeq.isSequence = true;
            consoleState.escapeSeq.key = EscapeKey::ArrowUp;
            break;
        case 'B':
            consoleState.escapeSeq.isSequence = true;
            consoleState.escapeSeq.key = EscapeKey::ArrowDown;
            break;
        case 'C':
            consoleState.escapeSeq.isSequence = true;
            consoleState.escapeSeq.key = EscapeKey::ArrowRight;
            break;
        case 'D':
            consoleState.escapeSeq.isSequence = true;
            consoleState.escapeSeq.key = EscapeKey::ArrowLeft;
            break;
        default:
            consoleState.escapeSeq.isSequence = false;
            consoleState.escapeSeq.key = EscapeKey::None;
            break;
        }
    }
}

ConsoleInput::ConsoleInput() {
    ConsoleInput::setRawMode();
    struct sigaction sa;

    sa.sa_handler = resizeHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGWINCH, &sa, NULL) == -1) {
        throw new std::runtime_error("sigaction");
    }
    atexit(reset);
    resizeHandler(SIGWINCH);
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
        updateConsoleState();
        if (resized) return '\0';
        if (nread == -1 && errno != EAGAIN) {
            if (errno == EINTR) {
                resized = true;
                return '\0';
            }
            ERR("ConsoleInput::get: read failed!");
        }
    }
    if (out == '\x1b')
        getEscapeSequence(out);
    return out;
}

char ConsoleInput::get(char& out) {
    out = get();
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
    setCursor({0, 0});
}

void ConsoleInput::setCursor(Coords pos) {
    std::stringstream ss;
    ss << "\x1b[" << (pos.y + 1) << ';' << (pos.x + 1) << 'f';
    std::string command = ss.str(); 
    
    writeToStdout(command.c_str(), command.length());
}

Coords ConsoleInput::getCursor() {
    return outerGetCursor();
}

ConsoleInput::ConsoleState ConsoleInput::getConsoleState() {
    updateConsoleState();
    return consoleState;
}

bool ConsoleInput::isInitialized() { return isRaw; }


#endif