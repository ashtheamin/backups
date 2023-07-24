#ifndef LIBSDEFS_INCLUDED
#define LIBSDEFS_INCLUDED
    #include "libsdefs.c"
#endif

#include "world.c"

int main(void) {
    struct world* world = world_init();
    if (world == NULL) {
        printf("main(): world_init() failed. Exiting.\n");
        return -1;
    }

    #if defined(__EMSCRIPTEN__)
        #include <emscripten.h>
        emscripten_set_main_loop_arg(world_loop, (void*)world, 0, 1);
    #else
        while (world->status != WORLD_STATUS_QUIT) {
            world_loop((void*)world);
        }
    #endif

    world_quit(world);

    return 0;
}