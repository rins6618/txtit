#ifndef PLATFORM__H
#define PLATFORM__H


#include <iostream>
#include <optional>
#include <string>
#include <cstdlib>
#include "types.h"

#define CTRL(x) (x & 0x1f)

class ConsoleInput {
public:
    struct ConsoleState{
        int cols;
        int rows;
        bool resized;
    };

private:

    void updateConsoleState();
    static bool isRaw;

    static void setRawMode();
    static void resetCanonicalMode();

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
