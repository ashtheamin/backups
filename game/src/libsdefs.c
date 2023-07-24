#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <SDL2/SDL.h>

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720


enum WORLD_STATUS {WORLD_STATUS_RUNNING, WORLD_STATUS_QUIT};

enum TILESET {
    TILE_AIR, TILE_WALL
};