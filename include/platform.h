#ifndef PLATFORM__H
#define PLATFORM__H


#include <iostream>
#include <optional>
#include <string>
#include <cstdlib>

#define CTRL(x) (x & 0x1f)

class ConsoleInput {
    static bool isRaw;

    static void setRawMode();
    static void resetCanonicalMode();
    
public:

    ConsoleInput();
    ~ConsoleInput();

    char get();
    char get(char& out);

    void writeToStdout(const char* msg, int bytes);
    void clearScreen();
    void resetCursor();

    static void reset();

    static bool isInitialized();
    


};

#endif // PLATFORM_H
