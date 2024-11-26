#ifndef PLATFORM__H
#define PLATFORM__H


#include <iostream>
#include <optional>
#include <string>
#include <cstdlib>


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

};

#endif // PLATFORM_H
