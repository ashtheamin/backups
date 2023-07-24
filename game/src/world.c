#ifndef LIBSDEFS_INCLUDED
#define LIBSDEFS_INCLUDED
    #include "libsdefs.c"
#endif

#include "player.c"
#include "tile.c"
#include "entity.c"
#include "camera.c"

struct world {
    SDL_Window* window;
    SDL_Renderer* renderer;
    enum WORLD_STATUS status;
    struct player player;
    struct tilemap* tilemap_list;
    struct entity* entity_list;
    struct camera camera;
};

struct world* world_init() {
    struct world* world = malloc(sizeof(struct world));
    if (world == NULL) {
        printf("world_init(): Failed to allocate memory for world. Exit.\n");
        return NULL;
    }

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("world_init(): Failed to initialise SDL.\n"
        "SDL_GetError(): %s.\n"
        "Exiting.\n", SDL_GetError());
        free(world);
        return NULL;
    }

    world->window = SDL_CreateWindow("Window",\
    SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,\
    SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

    if (world->window == NULL) {
        printf("world_init(): Failed to initialise SDL Window.\n"
        "SDL_GetError(): %s.\n"
        "Exiting.\n", SDL_GetError());
        SDL_Quit();
        free(world);
        return NULL;
    }

    world->renderer = SDL_CreateRenderer(world->window, -1, SDL_RENDERER_ACCELERATED);
    if (world->renderer == NULL) {
        printf("world_init(): Failed to initialise SDL Renderer.\n"
        "SDL_GetError(): %s.\n"
        "Exiting.\n", SDL_GetError());
        if (world->window != NULL) SDL_DestroyWindow(world->window);
        SDL_Quit();
        free(world);
        return NULL;
    }

    world->tilemap_list = tilemap_init("src/maps/map1");
    if (world->tilemap_list == NULL) {
         printf("world_init(): Failed to initialise tilemap.\n"
        "Exiting.\n");
        if (world->renderer != NULL) SDL_DestroyRenderer(world->renderer);
        if (world->window != NULL) SDL_DestroyWindow(world->window);
        SDL_Quit();
        free(world);
        return NULL;
    }

   
    // Load tiles into the world.
    world->entity_list = entity_init();
    struct entity* current_entity = world->entity_list;

    struct tilemap* tilemap = world->tilemap_list;

    for (int i=0; i < tilemap->num_rows; i++) {
        for (int j=0; j < tilemap->num_columns; j++) {
            current_entity->is_tile = true;
            current_entity->rect.w = tilemap->width;
            current_entity->rect.h = tilemap->height;
            current_entity->rect.x = j * tilemap->width;
            current_entity->rect.y = i * tilemap->height;
            current_entity->tile = tilemap->tileset[i][j];

            current_entity->next = entity_init();
            current_entity = current_entity->next;   
        }
    }

    world->player.x = 50;
    world->player.y = 50;
    world->player.velx = 0;
    world->player.vely = 0;
    world->player.velocity = 10;
    world->player.width = 0.2 * world->tilemap_list->width;
    world->player.height = 0.7 * world->tilemap_list->height;

    world->camera.x = 400;
    world->camera.y = 0;
    world->camera.speed = 40;

    world->status = WORLD_STATUS_RUNNING;
    return world;
}

void world_quit(struct world* world) {
    if (world == NULL) return;
    if (world->renderer != NULL) SDL_DestroyRenderer(world->renderer);
    if (world->window != NULL) SDL_DestroyWindow(world->window);
    if (world->tilemap_list != NULL) tilemap_list_free(world->tilemap_list);
    if (world->entity_list != NULL) entity_list_free(world->entity_list);
    SDL_Quit();
    free(world);
}

void world_input(struct world* world) {
    if (world == NULL) return;
    SDL_Event event;
    while (SDL_PollEvent(&event) != 0) {
        if (event.type == SDL_QUIT) {
            world->status = WORLD_STATUS_QUIT;
        }

        if (event.type == SDL_KEYDOWN) {
            switch(event.key.keysym.sym) {
                case SDLK_UP:
                    if (world->camera.y - world->camera.speed >= 0) {
                        world->camera.y = world->camera.y - world->camera.speed;
                    }
                    break;

                case SDLK_DOWN:
                    world->camera.y = world->camera.y + world->camera.speed;
                    break;

                case SDLK_LEFT:
                if (world->camera.x - world->camera.speed >= 0)
                    world->camera.x = world->camera.x - world->camera.speed;
                break;

                case SDLK_RIGHT:
                    world->camera.x = world->camera.x + world->camera.speed;
                    break;       
            }
        }
    }
}

void world_render(struct world* world) {
    if (world == NULL) return;

    SDL_SetRenderDrawColor(world->renderer, 0, 0, 0, 0);
    SDL_RenderClear(world->renderer);
    SDL_SetRenderDrawColor(world->renderer, 0, 128, 128, 0);

    struct entity* entity = world->entity_list;
    
    while (entity != NULL) {
        if (entity->is_tile == true) {
            if (entity->tile != TILE_AIR) {
                SDL_Rect rect = entity->rect;
                rect.x = rect.x - world->camera.x;
                rect.y = rect.y - world->camera.y;

                SDL_RenderFillRect(world->renderer, &rect);
            }
        }
        entity = entity->next;
    }
    SDL_Rect player_rect;
    player_rect.x = world->player.x;
    player_rect.y = world->player.y;
    player_rect.w = world->player.width;
    player_rect.h = world->player.height;

    SDL_SetRenderDrawColor(world->renderer, 255, 105, 180, 0);
    SDL_RenderFillRect(world->renderer, &player_rect);

    SDL_RenderPresent(world->renderer);
}

void world_loop(void* world_loop_argument) {
    struct world* world = (struct world*)world_loop_argument;
    if (world == NULL) return;
    world_input(world); 
    world_render(world);   
}