#include "../../graphics/src/engine/engine.h"

#define SDL_main main

//main entry point
int main(int argc, char* argv[]){
    MB_Engine engine;

    engine.init();

    engine.run();

    engine.cleanup();

    return 0;
}
