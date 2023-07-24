#ifndef LIBSDEFS_INCLUDED
#define LIBSDEFS_INCLUDED
    #include "libsdefs.c"
#endif

struct entity {
    SDL_Rect rect;
    bool is_tile;
    enum TILESET tile;
    struct entity* next;
};

struct entity* entity_init() {
    struct entity* entity = malloc(sizeof(struct entity));
    
    entity->rect.x = 0;
    entity->rect.y = 0;
    entity->rect.w = 0;
    entity->rect.h = 0;

    entity->is_tile = false;
    entity->tile = TILE_AIR;

    entity->next = NULL;
    return entity;
}

void entity_free(struct entity* entity) {
    if (entity == NULL) return;
    free(entity);
}

void entity_list_free(struct entity* entity_list) {
    if (entity_list == NULL) return;

    struct entity* entity = entity_list;
    struct entity* next;

    while (entity != NULL) {
        next = entity->next;
        free(entity);
        entity = next;
    }
}