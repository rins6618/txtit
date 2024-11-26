#include <iostream>
#include <fstream>

#include "platform.h"

int main() {

    ConsoleInput ci;

    char active {};
    while (!(ci.get(active) == EOF) and active != 'q') {

        if (iscntrl(active)) {
            std::cout << (int) active << std::endl;
        } else {
            std::cout << (int) active  << '(' << active << ')' << std::endl;
        }
    }

    return 0;
}