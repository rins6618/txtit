#ifndef PLATFORM__H
#define PLATFORM__H

#include <iostream>
#include <optional>
#include <string>
#include <cstdlib>
#include "types.h"

#define CTRL(x) (x & 0x1f)
#define UNUSED(x) (void)(x)

enum class EscapeKey {
    ArrowLeft,
    ArrowRight,
    ArrowUp,
    ArrowDown,
    None
};


class ConsoleInput {
public:
    struct EscapedSequence {
        bool isSequence;
        EscapeKey key;
    };

    struct ConsoleState{
        int cols;
        int rows;
        bool resized;
        EscapedSequence escapeSeq;
    };

private:

    void updateConsoleState();
    static bool isRaw;

    static void setRawMode();
    static void resetCanonicalMode();
    void getEscapeSequence(char out);

    ConsoleState consoleState;
    
public:

    ConsoleInput();
    ~ConsoleInput();

    char get();
    char get(char& out);

    void writeToStdout(const char* msg, int bytes);
    
    void clearScreen();
    void resetCursor();
    void setCursor(Coords pos);
    Coords getCursor();

    ConsoleState getConsoleState();

    static void reset();

    static bool isInitialized();
    


};

#endif // PLATFORM_H
