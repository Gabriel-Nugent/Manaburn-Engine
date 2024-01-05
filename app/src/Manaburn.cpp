#include "../../graphics/src/MB_Engine.h"

#define SDL_main main

//main entry point
int main(int argc, char* argv[]){
    GRAPHICS::MB_Engine engine;

    engine.init();

    engine.run();

    engine.cleanup();

    return 0;
}
