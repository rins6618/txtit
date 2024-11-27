#ifndef EDITOR_H
#define EDITOR_H

#include "platform.h"
#include "types.h"

class Editor {
    struct EditorConfiguration {
        Coords cursor;
        Coords dimensions;
    } ec;
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