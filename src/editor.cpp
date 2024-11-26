#include "editor.h"
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
    std::cout << "Quitting editor...\n";
}

void Editor::editorLoop() {
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
    default:
        break;
    }
}