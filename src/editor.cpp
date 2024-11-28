#include "editor.h"
#include "platform.h"
#include "txtit.h"
#include <iostream>
#include <sstream>

Editor::Editor(ConsoleInput& _ci_instance) : ci_instance(_ci_instance) {
    ec.cursor = {0, 0};
    if (!ConsoleInput::isInitialized()) {
        throw new std::runtime_error("Console not prepped");
    }
    running = true;
    editorLoop();

}

Editor::~Editor() {
    ConsoleInput::reset();
    ci_instance.clearScreen();
    std::cout << "Quitting editor...\n";
}

void Editor::rowIndicators() {
    auto cs = ci_instance.getConsoleState();
    ec.dimensions.x = cs.cols;
    ec.dimensions.y = cs.rows;
    int numLen = ((int) std::to_string(ec.dimensions.x).length()) + 1;


    std::stringstream outersstr;
    for (int i = 1; i <= ec.dimensions.y; ++i) {
        std::stringstream innersstr;
        
        if (i == ec.dimensions.y / 3) {
            std::stringstream splashMsg;
            splashMsg << "txtit -- text editor -- v" << txtit_VERSION_MAJOR << '.' << txtit_VERSION_MINOR;
            std::string splashStr = splashMsg.str();
            int padding = (int) (ec.dimensions.x - splashStr.length()) / 2;
            if (padding) {
                innersstr << i;
                int iLen = (int) innersstr.str().length();
                while (iLen < numLen) {
                    innersstr << ' ';
                    iLen++;
                }
                innersstr << "#";
                padding -= numLen + 1;
            }
            while (padding--) innersstr << " ";
            innersstr << splashStr;
        } else if (i == ec.dimensions.y / 3 + 1) {
            std::stringstream tooltip;
            tooltip << "CTRL+Q to quit";
            std::string tooltipStr = tooltip.str();
            int padding = (cs.cols - tooltipStr.length()) / 2;
            if (padding) {
                innersstr << i;
                int iLen = (int) innersstr.str().length();
                while (iLen < numLen) {
                    innersstr << ' ';
                    iLen++;
                }
                innersstr << "#";
                padding -= numLen + 1;
            }
            while (padding--) innersstr << " ";
            innersstr << tooltipStr;
        } else {
            innersstr << i;
            int iLen = (int) innersstr.str().length();
            while (iLen < numLen) {
                innersstr << ' ';
                iLen++;
            }
            innersstr << "#";
        }
        if (i < ec.dimensions.y) innersstr << "\r\n";
        outersstr << innersstr.str();
    }
    std::string str = outersstr.str();
    ci_instance.writeToStdout(str.c_str(), (int) str.length());
}

void Editor::editorLoop() {
    ci_instance.clearScreen();
    rowIndicators();
    ci_instance.resetCursor();
    while(running) {
        processKey();    
    }
}

void Editor::processKey() {
    char active = ci_instance.get();
    if (active == '\x1b') {
        std::string chord;
        chord.push_back(ci_instance.get());
        chord.push_back(ci_instance.get());

        if (chord.at(0) == '[') {
            switch (chord[1]) {
                case 'A':
                    ci_instance.setCursor({ec.cursor.x, --ec.cursor.y});
                    break;
                case 'B':
                    ci_instance.setCursor({ec.cursor.x, ++ec.cursor.y});
                    break;
                case 'C':
                    ci_instance.setCursor({++ec.cursor.x, ec.cursor.y});
                    break;
                case 'D':
                    ci_instance.setCursor({--ec.cursor.x, ec.cursor.y});
                    break;
                default:
                    break;
            }
        }

    }
    
    switch (active)
    {
    case CTRL('Q'):
        running = false;
        ci_instance.clearScreen();
        break;
    case CTRL('L'):
        ci_instance.clearScreen();
        break;
    case '\0':
        if (ci_instance.getConsoleState().resized) {
            ci_instance.clearScreen();
            rowIndicators();
            ci_instance.resetCursor();
        }
        break;
    default:
        break;
    }
}