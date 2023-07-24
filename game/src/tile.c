#ifndef LIBSDEFS_INCLUDED
#define LIBSDEFS_INCLUDED
    #include "libsdefs.c"
#endif


struct tilemap {
    int** tileset;
    int num_rows;
    int num_columns;
    int width;
    int height;
    struct tilemap* next;
};

struct tilemap* tilemap_init(char* filename) {
    struct tilemap* tilemap = malloc(sizeof(struct tilemap));
    if (tilemap == NULL) {
        printf("tilemap_init(): Failed to init tilemap. Exit.\n");
        return NULL;
    }

    FILE* fp = fopen(filename, "r+");
    if (fp == NULL) {
        printf("Failed to open file '%s'. Exiting.\n", filename);
        free(tilemap);
        return NULL;
    }

    char* buffer = malloc(sizeof(char) * 4096);

    size_t columns = 0;
    size_t count = 0;
    while (fgets(buffer, 4096, fp) != NULL) {
        buffer[strcspn(buffer, "\n")] = 0;
        if (count == 0) columns = strlen(buffer);
        if (count != 0) {
            if (columns != strlen(buffer)) {
                free(buffer);
                printf("tilemap_init(): Inconsistent map dimensions. Exiting.\n");
                fclose(fp);
                free(tilemap);
                return NULL;
            }
        }
        count++;
    }
    fclose(fp);
    free(buffer);

    tilemap->num_rows = count;
    tilemap->num_columns = columns;

    fp = fopen(filename, "r+");
    if (fp == NULL) {
        printf("Failed to open file '%s'. Exiting.\n", filename);
        free(tilemap);
        return NULL;
    }

    tilemap->tileset = (int**)malloc(tilemap->num_rows * sizeof(int*));
    for (int i = 0; i < tilemap->num_rows; i++) {
        tilemap->tileset[i] = (int*)malloc(tilemap->num_columns * sizeof(int));
    }

    buffer = malloc(sizeof(char) * 4096);
    size_t row = 0;

    while (fgets(buffer, 4096, fp) != NULL) {
        buffer[strcspn(buffer, "\n")] = 0;
        int current_column = 0;
        while (current_column < tilemap->num_columns) {
            switch(buffer[current_column]) {
                case 'A':
                    tilemap->tileset[row][current_column] = TILE_AIR;break;
                case 'W':
                    tilemap->tileset[row][current_column] = TILE_WALL;break;
                default:
                    printf("tilemap_init(): Invalid tile read. Returning\n");
                    for (int i=0; i < tilemap->num_rows; i++) {
                        free(tilemap->tileset[i]);
                    }
                    free(tilemap->tileset);
                    free(tilemap);
                    fclose(fp);
                    free(buffer);
                    return NULL;
            }
            current_column++;
        }
        row++;
    }
    free(buffer);
    fclose(fp);

    tilemap->next = NULL;
    tilemap->width = 80;
    tilemap->height = 80;
    return tilemap;
}

void tilemap_free(struct tilemap* tilemap) {
    if (tilemap == NULL) return;
    for (int i=0; i < tilemap->num_rows; i++) {
        free(tilemap->tileset[i]);
    }
    free(tilemap->tileset);
    free(tilemap);
}

void tilemap_list_free(struct tilemap* tilemap_list) {
    if (tilemap_list == NULL) return;
    struct tilemap* tilemap = tilemap_list;
    struct tilemap* next;

    while (tilemap != NULL) {
        next = tilemap->next;
        tilemap_free(tilemap);
        tilemap = next;
    }
}