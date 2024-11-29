#include <iostream>
#include <fstream>

#include "platform.h"
#include "editor.h"

int main() {

    ConsoleInput ci;
    try {
        Editor editor(ci);
    } catch(const std::runtime_error* re) {
        ci.clearScreen();
        ci.reset();
        std::cerr << "Exception in txtit: => " << re->what() << std::endl;
    }

    return 0;
}