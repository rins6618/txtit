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

    char get();
    char get(char& out);

    ConsoleInput();
    ~ConsoleInput();
    static void reset();

    static bool isInitialized();

};

#endif // PLATFORM_H
