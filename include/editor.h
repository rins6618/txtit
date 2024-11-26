#ifndef EDITOR_H
#define EDITOR_H

#include "platform.h"

class Editor {
    ConsoleInput ci_instance;
    bool running;

public:
    Editor(ConsoleInput& _ci_instance);
    ~Editor();
    void rowIndicators();
    void editorLoop();
    void processKey();
    char readKey();
};

#endif