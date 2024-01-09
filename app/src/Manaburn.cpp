#include "../../graphics/src/MB_Engine.h"

#define SDL_main main

//main entry point
int main(int argc, char* argv[]){
    GRAPHICS::MB_Engine MB_Engine;

    MB_Engine.init();

    MB_Engine.run();

    MB_Engine.cleanup();

    return 0;
}
