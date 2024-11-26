#include "editor.h"
#include "platform.h"
#include <iostream>

Editor::Editor(ConsoleInput& _ci_instance) : ci_instance(_ci_instance) {
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
    for (int i = 0; i < 24; ++i) {
        ci_instance.writeToStdout("#\r\n", 3);
    }
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
    switch (active)
    {
    case CTRL('q'):
        running = false;
        break;
    case CTRL('L'):
        ci_instance.clearScreen();
        break;
    default:
        break;
    }
}