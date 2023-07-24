// Snek: A simple video game by Ash Amin (Copyright 2022)

// Include necessary libraries
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#ifdef __EMSCRIPTEN__
    #include <emscripten/emscripten.h> 
#endif

// Define screen related constants.
// This will be based around trying to have cells that are close to a square as possible.
// This is based on a 16:9 display aspect ratio and settings.
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define MAP_ROWS 30
#define MAP_COLUMNS 53

// Define program status constants:
#define START_MENU 0
#define MID_GAME 1
#define GAME_OVER 2
#define QUIT_LOOP 3
#define PAUSE 4

// Define constants for direction:
#define UP 0
#define DOWN 1
#define LEFT 2
#define RIGHT 3

// Define constants for colour labels for tiles:
#define BLACK 0
#define GREEN 1
#define RED 2
#define GREY 3
#define HEAD 4

// Define constants for difficulty:
// This is equivalent to the milliseconds between updates.
#define EASY 100
#define REGULAR 50 
#define HARD 30

// Create a linked list data type to represent the snek entity.
struct snek_entity {
    int32_t row;
    int32_t column;

    struct snek_entity* next;
};

// Create a global data type to hold all associated data with the program.
struct snek {
    // SDL data:
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Event event;

    // Entity data:
    struct snek_entity* head;
    int32_t direction;
    int32_t score;

    int32_t food_row;
    int32_t food_column;

    // Tile Map data:
    int32_t map[MAP_ROWS][MAP_COLUMNS];

    // Program status:
    int32_t status;

    // Set timer storage:
    int32_t current_time;
    int32_t last_time;

    // Font data:
    TTF_Font* font;

    // Difficulty:
    int32_t difficulty;
};

// Global Variables:
// A global pointer to an allocated instance of the snek program on the heap.
struct snek* snek = NULL;

// Return a pointer to a new instance of a snek entity node with the passed in row and column with information
// Returns pointer on success, and returns NULL on failure.
struct snek_entity* snek_entity_new(int32_t row, int32_t column) {
    // Allocate memory for the new node.
    struct snek_entity* new_snek_entity = NULL;
    new_snek_entity = (struct snek_entity*)malloc(sizeof(struct snek_entity));

    // Return an error and false if memory allocation failed.
    if (new_snek_entity == NULL) {
        printf("snek_entity_new(): Failed to allocate memory to create a new snek entity node. Returning NULL.\n");
        return NULL;
    }

    // Set passed in and appropriate values for the new node.
    new_snek_entity->row = row;
    new_snek_entity->column = column;
    new_snek_entity->next = NULL;

    // Return a pointer to the new snek entity allocated to the heap.
    return new_snek_entity;
}

// Create a new node at the end of a snek entity using passed in functions.
// Returns true on success, and false on failure.
// On failure, it will not modify the passed in entity.
bool snek_entity_append(struct snek_entity* snek_entity, int32_t row, int32_t column) {
    // If passed in snek entity pointer is NULL, return failure.
    if (snek_entity == NULL) {
        printf("snek_entity_append(): Snek entity passed into function is NULL. Returning false.\n");
        return false;
    }

    // Find the tail node of the snek entity passed into the function.
    struct snek_entity* temp = snek_entity;
    while (temp->next != NULL) {
        temp = temp->next;
    }

    // Allocate a new node to heap at the end of the tail node of the passed in snek entity.
    // Return an error on failure.
    temp->next = snek_entity_new(row, column);
    if (temp->next == NULL) {
        printf("snek_entity_append(): Failed to create new node at the end of the function. Returning false.\n");
        return false;
    }

    // Return true on success.
    return true;
}

// Free all memory allocated to a passed in snek entity from that node onwards.
// Ensure the head of the snek entity is passed in, so all memory is freed.
bool snek_entity_free(struct snek_entity* snek_entity) {
    // Return with error if snek entity passed into function is NULL.
    if (snek_entity == NULL) {
        printf("snek_entity_free(): Snek entity passed into function is equal to NULL. Returning false.\n");
        return false;
    }

    // Set pointer variables to store memory locations of nodes on the heap.
    struct snek_entity* temp = snek_entity;
    struct snek_entity* next = NULL;

    // Traverse through the snek entity linked list.
    // Store the value of the next node, free the current node, and go to the next node until all memory is freed.
    while (temp != NULL) {
        next = temp->next;
        free(temp);
        temp = next;
    }

    // Return true on success.
    return true;
}

// Print all nodes within a snek entity.
// Return true on ability to print, return false on error.
bool snek_entity_print(struct snek_entity* snek_entity) {
    // If passed in snek entity pointer is NULL, return.
    if (snek_entity == NULL) {
        printf("snek_entity_print(): Snek entity passed into function is equal to NULL. Returning false.\n");
        return false;
    }

    // Traverse through the snek entity and print all the nodes.
    // Keep a count of what number each node is.
    struct snek_entity* temp = snek_entity;
    int32_t node_counter = 0;

    while (temp != NULL) {
        printf("Node: %d Row: %d Column: %d\n", node_counter, temp->row, temp->column);
        temp = temp->next;
        node_counter++;
    }

    // Return true on success.
    return true;
}

// Spawn a new instance of a food entity:
// Ensure it is outside wherever the snek entity exists.
bool snek_food_entity_spawn() {
    // Return if the global entity pointer does not point to a valid location on heap.
    if (snek == NULL) {
        printf("snek_food_entity_spawn(): Global snek variable is NULL. Returning false.\n");
        return false;
    }

    // There is no need to check if the snek entity is NULL or valid.
    // That is because if it is equal to NULL, then the loop will be executed once and exit.
    int32_t food_is_inside_snek = 1;
    struct snek_entity* temp = snek->head;
    
    // Keep trying to spawn a random location for the food within bounds of the tile map.
    // Ensure it is outside every node in the snek entity.

    // If the function is taking too long, then quit after 10 seconds.
    // This is due to the randomness of trying to find a solution.
    // This is to prevent the program hanging for too long if something goes wrong.
    int32_t current_time = SDL_GetTicks();
    int32_t last_time = current_time;

    srand(time(0));
    while (food_is_inside_snek == 1) {
        food_is_inside_snek = 0;

        // Calculate new food position
        // Spawn the food inside the designated zone for the gameplay.
        // This is because there are defined borders for the game around the edges.
        snek->food_row = rand() % ((MAP_ROWS - 2) + 1 - 2) + 2;
        snek->food_column = rand() % ((MAP_COLUMNS - 2) + 1 - 1) + 1;

        // Traverse through the snek entity and every node.
        // If any node matches the position of the food entity, try again.
        while (temp != NULL) {
            if (temp->row == snek->food_row && temp->column == snek->food_column) {
                food_is_inside_snek = 1;
            }
            temp = temp->next;
        }

        // If enough time has elapsed since the loop has been entered, this means the function is taking too long.
        // Give up trying to search for a new food location and quit the loop.
        current_time = SDL_GetTicks();
        if (current_time > last_time + 10000) {
            printf("snek_food_entity_spawn(): Function is taking too long to find a solution. Returning false.\n");
            return false;
        }
    }

    // At this point, a valid food location that isn't intersecting with a snek entity should have been successfully found.
    // Return true.
    return true;
}

// Render text to location on renderer:
bool snek_render_text(char* text, int32_t x, int32_t y, int32_t w, int32_t h) {
    // Return if the global entity pointer does not point to a valid location on heap.
    if (snek == NULL) {
        printf("snek_food_entity_spawn(): Global snek variable is NULL. Returning false.\n");
        return false;
    }

    if (text == NULL) {
        printf("snek_food_entity_spawn(): Invalid text pointer. Returning false.\n");
        return false;
    }

    // Create the score string texture
    SDL_Color white = {255, 255, 255};
    SDL_Surface* text_surface = TTF_RenderText_Solid(snek->font, text, white);
    SDL_Texture* text_texture = SDL_CreateTextureFromSurface(snek->renderer, text_surface);

    // Set the position the texture will be put in the screen.
    SDL_Rect text_rect;
    text_rect.x = x;
    text_rect.y = y;
    text_rect.w = w;
    text_rect.h = h;

    // Copy the score texture to the renderer.
    SDL_RenderCopy(snek->renderer, text_texture, NULL, &text_rect);

    SDL_DestroyTexture(text_texture);

    return true;
}

// Initialise the tile map that is the world that the entities reside/exist in.
bool snek_map_init() {
    // Return failure if the global entity pointer does not point to a valid location on heap.
    if (snek == NULL) {
        printf("snek_map_init(): Snek global variable is NULL. Returning false.\n");
        return false;
    }

    // Set all tiles on the tile map to black.
    for (int32_t i = 0; i < MAP_ROWS; i++) {
        for (int32_t j = 0; j < MAP_COLUMNS; j++) {
            snek->map[i][j] = BLACK;
        }
    }

    // Set the designated wall tiles.
    // These are the first two rows, the last row.
    // They are also the first and last column:
    for (int32_t i = 0; i < MAP_COLUMNS; i++) {
        snek->map[0][i] = GREY;
        snek->map[1][i] = GREY;
        snek->map[MAP_ROWS-1][i] = GREY;
    }

    for (int32_t i = 0; i < MAP_ROWS; i++) {
        snek->map[i][0] = GREY;
        snek->map[i][MAP_COLUMNS-1] = GREY;
    }


    // Return success.
    return true;
}

// Initialise the global snek instance:
// Return true on success, and false on failure.
bool snek_init() {
    // Set the snek pointer global variable to point to a valid allocated chunk of memory in the heap.
    // Return on failure to do so.
    snek = (struct snek*) malloc(sizeof(struct snek));
    if (snek == NULL) {
        printf("snek_init(): Malloc failed when trying to assign memory to snek global pointer. Returning false.\n");
        return false;
    }

    // Attempt to initialise SDL.
    // // Return failure on failure to do so and free all allocated resources.
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("snek_init(): Failed to initialise SDL. SDL_GetError(): %s. Returning false.\n", SDL_GetError());
        free(snek);
        snek = NULL;
        return false;
    }

    // Attempt to initialise SDL Window.
    // // Return failure on failure to do so and free all allocated resources.
    snek->window = SDL_CreateWindow("Snek", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (snek->window == NULL) {
        printf("snek_init(): Failed to initialise SDL Window. SDL_GetError(): %s. Returning false.\n", SDL_GetError());
        SDL_Quit();
        free(snek);
        snek = NULL;
        return false;
    }

    // Attempt to initialise SDL Renderer. 
    // Return failure on failure to do so and free all allocated resources.
    snek->renderer = SDL_CreateRenderer(snek->window, -1, SDL_RENDERER_ACCELERATED);
    if (snek->renderer == NULL) {
        printf("snek_init(): Failed to initialise SDL Window. SDL_GetError(): %s. Returning false.\n", SDL_GetError());
        SDL_DestroyWindow(snek->window);
        SDL_Quit();
        free(snek);
        snek = NULL;
        return false;
    }

    // Attempt to initialise snek entity. 
    // Return failure on failure to do so and free all allocated resources.
    // Spawn the snek entity in the middle of the map or screen.
    snek->head = snek_entity_new(MAP_ROWS/2, MAP_COLUMNS/2);
    if (snek->head == NULL) {
        printf("snek_init(): snek_entity_new() failed to create snek entity for program. Returning false.\n");
        SDL_DestroyRenderer(snek->renderer);
        SDL_DestroyWindow(snek->window);
        SDL_Quit();
        free(snek);
        snek = NULL;
        return false;
    }

    // Initialise the tile map.
    // Return failure on failure to do so and free all allocated resources.
    if (snek_map_init() == false) {
        printf("snek_init(): snek_map_init() failed to initialise map for program. Returning false.\n");
        SDL_DestroyRenderer(snek->renderer);
        SDL_DestroyWindow(snek->window);
        SDL_Quit();
        snek_entity_free(snek->head);
        free(snek);
        snek = NULL;
        return false;        
    }

    // Spawn the food in a random unique location away from the snek entity
    // Return failure on failure to do so and free all allocated resources.
    if (snek_food_entity_spawn() == false) {
        printf("snek_init(): snek_food_entity_spawn() failed to find food spawn location for program. Returning false.\n");
        SDL_DestroyRenderer(snek->renderer);
        SDL_DestroyWindow(snek->window);
        SDL_Quit();
        snek_entity_free(snek->head);
        free(snek);
        snek = NULL;
        return false;    
    }

    // Initialise TTF Engine:
    // Return failure on failure to do so and free all allocated resources.
    if (TTF_Init() != 0) {
        printf("snek_init(): Failed to intialise TTF rendering engine. Returning false.\n");
        SDL_DestroyRenderer(snek->renderer);
        SDL_DestroyWindow(snek->window);
        SDL_Quit();
        snek_entity_free(snek->head);
        free(snek);
        snek = NULL;
        return false;  
    }

    // Open TTF font.
    // Return failure on failure to do so and free all allocated resources.
    snek->font = TTF_OpenFont("third_party/roboto_mono/RobotoMono-Bold.ttf", 24);
    if (snek->font == NULL) {
        printf("snek_init(): Failed to open TTF font. Returning false.\n");
        SDL_DestroyRenderer(snek->renderer);
        SDL_DestroyWindow(snek->window);
        SDL_Quit();
        TTF_Quit();
        snek_entity_free(snek->head);
        free(snek);
        snek = NULL;
        return false; 
    }

    // Assign a default direction for the snek entity.
    // This is just an initialisation step, in practise a user's input will be what is assigned.
    snek->direction = UP;

    // Set the program status to start, which means that it will wait for user input before beginning the gameplay.
    snek->status = START_MENU;

    // Initialise timer.
    snek->last_time = 0;

    // Initialise score:
    snek->score = 1;

    // Initialise Difficulty:
    snek->difficulty = REGULAR;

    // Return true if all initialisation steps have succeeded.
    return true;
}

// Quit SDL and free all memory allocated to snek on heap.
// Return true on success, and false on failure.
bool snek_quit() {
    // Return false if the global variable snek pointer is set to NULL.
    if (snek == NULL) {
        printf("snek_quit(): Snek global variable is equal to NULL. Returning false.\n");
        return false;
    }

    // Free resources associated with SDL and quit SDL.
    SDL_DestroyRenderer(snek->renderer);
    SDL_DestroyWindow(snek->window);
    SDL_Quit();

    // Quit TTF renderer.
    TTF_Quit();

    // Free all memory assigned to snek entity.
    if (snek->head != NULL) {
        snek_entity_free(snek->head);
    }

    // Free the snek global variable from the heap.
    free(snek);

    // Return true.
    return true;
}

// Render the tilemap onto the screen.
// Return true on success, and false on failure.
bool snek_render() {
    // Return false if the snek global variable pointer does not point to a valid memory location on heap.
    if (snek == NULL) {
        printf("snek_render(): Snek global variable pointer is NULL. Returning false.\n");
        return false;
    }

    // Set the screen to grey.
    SDL_SetRenderDrawColor(snek->renderer, 32, 32, 32, 0);
    SDL_RenderClear(snek->renderer);

    // Initialise a rect for SDL to draw with, with appropriate position and dimensions
    SDL_Rect render_rect;
    render_rect.w = SCREEN_WIDTH/MAP_COLUMNS;
    render_rect.h = SCREEN_HEIGHT/MAP_ROWS;
    render_rect.x = 0;
    render_rect.y = 0;

    // Traverse through the tile map.
    // Get the render colour and calculate position to draw a tile of said colour and draw it for each tile.
    for (int32_t i=0; i < MAP_ROWS; i++) {
        for (int32_t j=0; j < MAP_COLUMNS; j++) {
            render_rect.x = j * render_rect.w;
            render_rect.y = i * render_rect.h;

            switch (snek->map[i][j]) {
                // Skip calling black tile renders since they were already rendered in the clear.
                case BLACK:
                    SDL_SetRenderDrawColor(snek->renderer, 0, 0, 0, 0);
                    SDL_RenderFillRect(snek->renderer, &render_rect);
                    break;

                case HEAD:
                    SDL_SetRenderDrawColor(snek->renderer, 0, 200, 20, 0);
                    SDL_RenderFillRect(snek->renderer, &render_rect);
                    break;

                case GREEN:
                    SDL_SetRenderDrawColor(snek->renderer, 0, 200, 60, 0);
                    SDL_RenderFillRect(snek->renderer, &render_rect);
                    break;

                case GREY:
                    SDL_SetRenderDrawColor(snek->renderer, 32, 32, 32, 0);
                    SDL_RenderFillRect(snek->renderer, &render_rect);
                    break;

                case RED:
                    SDL_SetRenderDrawColor(snek->renderer, 255, 0, 0, 0);
                    SDL_RenderFillRect(snek->renderer, &render_rect);
                    break;
            }
        }
    }

    // Display the current score!
    char *score = (char*) malloc(sizeof(char) * 4096);
    sprintf(score, "Score: %d", snek->score);
    snek_render_text(score, 0, 0, SCREEN_WIDTH/8, (SCREEN_HEIGHT/MAP_COLUMNS)*4);
    free(score);
    
    // Display the results on the screen and return true.
    SDL_RenderPresent(snek->renderer);
    return true;
}

// Render the main menu.
// Return true on success, and false on failure.
bool snek_render_menu() {
    // Return false if the snek global variable pointer does not point to a valid memory location on heap.
    if (snek == NULL) {
        printf("snek_render(): Snek global variable pointer is NULL. Returning false.\n");
        return false;
    }

    // Set the screen to dark grey
    SDL_SetRenderDrawColor(snek->renderer, 32, 32, 32, 0);
    SDL_RenderClear(snek->renderer);

    // Render the Title
    snek_render_text("Snek", 0, 0, SCREEN_WIDTH/8, (SCREEN_HEIGHT/MAP_COLUMNS)*4);

    // Render the difficulty settings based on difficulty selected.
    if (snek->difficulty == EASY) {
        snek_render_text("Difficulty selected: Easy", 0, ((SCREEN_HEIGHT/MAP_COLUMNS)*4), SCREEN_WIDTH/2, (SCREEN_HEIGHT/MAP_COLUMNS)*4);
    }

    if (snek->difficulty == REGULAR) {
        snek_render_text("Difficulty selected: Regular", 0, ((SCREEN_HEIGHT/MAP_COLUMNS)*4), SCREEN_WIDTH/2, (SCREEN_HEIGHT/MAP_COLUMNS)*4);
    }
    
    if (snek->difficulty == HARD) {
        snek_render_text("Difficulty selected: Hard", 0, ((SCREEN_HEIGHT/MAP_COLUMNS)*4), SCREEN_WIDTH/2, (SCREEN_HEIGHT/MAP_COLUMNS)*4);
    }

    snek_render_text("Press E for (E)asy R for (R)egular and Q for (Q)ard) difficulty. Press any other key to start.", 0, ((SCREEN_HEIGHT/MAP_COLUMNS)*4*2), SCREEN_WIDTH, (SCREEN_HEIGHT/MAP_COLUMNS)*4);
    
    SDL_RenderPresent(snek->renderer);
    return true;
}

// Render the game over screen.
// Return true on success, and false on failure.
bool snek_render_game_over() {
    // Return false if the snek global variable pointer does not point to a valid memory location on heap.
    if (snek == NULL) {
        printf("snek_render(): Snek global variable pointer is NULL. Returning false.\n");
        return false;
    }

    // Set the screen to dark grey
    SDL_SetRenderDrawColor(snek->renderer, 32, 32, 32, 0);
    SDL_RenderClear(snek->renderer);

    // Render game over.
    snek_render_text("Game Over!", 0, 0, SCREEN_WIDTH/2, (SCREEN_HEIGHT/MAP_COLUMNS)*4);

    // Render final score:
    char* final_score = (char* )malloc(sizeof(char) * 4096);
    sprintf(final_score, "Final Score: %d", snek->score);
    snek_render_text(final_score, 0, ((SCREEN_HEIGHT/MAP_COLUMNS)*4), SCREEN_WIDTH/2, (SCREEN_HEIGHT/MAP_COLUMNS)*4);
    free(final_score);

    // Render difficulty:
    if (snek->difficulty == EASY) {
        snek_render_text("Difficulty: Easy", 0, ((SCREEN_HEIGHT/MAP_COLUMNS)*4*2), SCREEN_WIDTH/2, (SCREEN_HEIGHT/MAP_COLUMNS)*4);
    }

    if (snek->difficulty == REGULAR) {
        snek_render_text("Difficulty: Regular", 0, ((SCREEN_HEIGHT/MAP_COLUMNS)*4*2), SCREEN_WIDTH/2, (SCREEN_HEIGHT/MAP_COLUMNS)*4);
    }

    if (snek->difficulty == HARD) {
        snek_render_text("Difficulty: Hard", 0, ((SCREEN_HEIGHT/MAP_COLUMNS)*4*2), SCREEN_WIDTH/2, (SCREEN_HEIGHT/MAP_COLUMNS)*4);
    }

    // Display the results on the screen:
    SDL_RenderPresent(snek->renderer);

    return true;
}

// Update the world and entities.
// Update entities first, then represent them correctly on the tile map.
// Return true on success, and false on failure.
bool snek_update() {
    // Return false if the snek global variable pointer does not point to a valid memory location on heap.
    if (snek == NULL) {
        printf("snek_update(): Snek global variable pointer is NULL. Returning false.\n");
        return false;
    }

    if (snek->head == NULL) {
        printf("snek_update(): Snek entity is NULL/invalid. Returning false.\n");
        return false;
    }

    // Reset the map:
    snek_map_init();

    // Create a new snek entity that will contain the updated data
    struct snek_entity* new_snek_entity;

    // Update the snek's head position based on what direction it is set to go.
    // If it goes out of bounds, return false and exit from the function
    switch (snek->direction) {
        case UP:
            if (snek->head->row - 1 <= 1) {
                return false;
            }
            new_snek_entity = snek_entity_new(snek->head->row - 1, snek->head->column);
            break;

        case DOWN:
            if (snek->head->row + 1 >= MAP_ROWS - 1) {
                return false;
            }
            new_snek_entity = snek_entity_new(snek->head->row + 1, snek->head->column);
            break;

        case LEFT:
            if (snek->head->column - 1 < 1) {
                return false;
            }
            new_snek_entity = snek_entity_new(snek->head->row, snek->head->column - 1);
            break;

        case RIGHT:
            if (snek->head->column + 1 >= MAP_COLUMNS-1) {
                return false;
            }
            new_snek_entity = snek_entity_new(snek->head->row, snek->head->column + 1);
            break;
    }

    // If there was a failure to return a pointer to a new snek entity head, return false.
    if (new_snek_entity == NULL) {
        printf("snek_update(): Failed to create a new snek entity for the updated snek entity. Returning false.\n");
        return false;
    }

    // Check to see if the snek entity consumed a food entity that round.
    // If so, set a flag that indicates it did and find a new location to spawn the food.
    bool food_consumed = false;
    if (new_snek_entity->row == snek->food_row && new_snek_entity->column == snek->food_column) {
        food_consumed = true;
    }

    // Copy the nodes from the snek entity that existed prior to updating position to the new snek entity
    // Leave out the tail node unless food has been consumed.
    // Since the head has already been updated, that means that copying the rest of the nodes from the old one will put them in the correct positions for the new snek entity, and copying the last node from the old snek entity will have the effect of growing the new snek in the right place.
    
    struct snek_entity* temp = snek->head;
    while (temp != NULL) {
        // Tail node check. Exit the loop if food has not been consumed and it is on the tail node.
        if (temp->next == NULL && food_consumed == false) {
            break;
        }

        // Append the old snek node to the new snek entity. Return from function on failure.
        if (snek_entity_append(new_snek_entity, temp->row, temp->column) == false) {
            printf("snek_update(): Failed to append new node to the new snek. Returning false.\n");
            snek_entity_free(new_snek_entity);
            return false;
        }

        // Traverse through the entity.
        temp = temp->next;
    }

    // If copying values to new snek entity occurs successfully, free old snek entity and set the snek entity pointer to the new updated one.
    snek_entity_free(snek->head);
    snek->head = new_snek_entity;

    // Check for snek entity head to body node collisions. If there are any nodes equal to the head, return false.
    // Traverse the new snek entity to find this out.
    // There is no error to be reported since this is not abnormal behaviour, it is an expected feature, a snek entity should not be allowed to eat itself. Hence the lack of printf().
    temp = snek->head;
    while (temp != NULL) {
        if (temp->row == snek->head->row && temp->column == snek->head->column && temp != snek->head) {
            return false;
        }
        temp = temp->next;
    }

    // Find a new food location that exists outside the snek entity:
    // This is only if the food is consumed.
    // Add a point to the score.
    if (food_consumed == 1) {
        if (snek_food_entity_spawn() == false) {
            printf("snek_update(): snek_food_entity_spawn returned false when attempting to find new food location to spawn. Return false.\n");
            return false;
        }
        snek->score++;
    }

    // Copy the snek and food entities to the tile map:
    temp = snek->head;
    while (temp != NULL) {
        snek->map[temp->row][temp->column] = GREEN;
        temp = temp->next;
    }
    snek->map[snek->head->row][snek->head->column] = HEAD;
    snek->map[snek->food_row][snek->food_column] = RED;

    // Return true since all went well.
    return true;
}

// Update the snek entity's direction based on input:
// Return true on success, and false on failure.
bool snek_input() {
    // Return false if the snek global variable pointer does not point to a valid memory location on heap.
    if (snek == NULL) {
        printf("snek_input(): Snek global variable pointer is NULL. Returning false.\n");
        return false;
    }

    // Set the direction based on recieved keypress
    switch (snek->event.key.keysym.sym) {
        case SDLK_w:
            if (snek->direction != DOWN || snek->status == START_MENU) {
                snek->direction = UP;
            }
            break;
        
        case SDLK_UP:
            if (snek->direction != DOWN || snek->status == START_MENU) {
                snek->direction = UP;
            }
            break;

        case SDLK_s:
            if (snek->direction != UP || snek->status == START_MENU) {
                snek->direction = DOWN;
            }
            break;

        case SDLK_DOWN:
            if (snek->direction != UP || snek->status == START_MENU) {
                snek->direction = DOWN;
            }
            break;

        case SDLK_d:
            if (snek->direction != LEFT || snek->status == START_MENU) {
                snek->direction = RIGHT;
            }
            break;
        
        case SDLK_RIGHT:
            if (snek->direction != LEFT || snek->status == START_MENU) {
                snek->direction = RIGHT;
            }
            break;

        case SDLK_a:
            if (snek->direction != RIGHT || snek->status == START_MENU) {
                snek->direction = LEFT;
            }
            break;
        
        case SDLK_LEFT:
            if (snek->direction != RIGHT || snek->status == START_MENU) {
                snek->direction = LEFT;
            }
            break;
    }

    // Return true on success.
    return true;
}

// Run the main program loop.
// Return true on time to quit, false to continue
void snek_loop() {
    // Return false if the snek global variable pointer does not point to a valid memory location on heap.
    if (snek == NULL) {
        printf("snek_loop(): Snek global variable pointer is NULL. Quitting.\n");
        return;
    }

    // Save some CPU time by pausing for a little.
    SDL_Delay(2);

    // Run the loop program procedure for before the game starts.
    // Start the game once the user is ready.
    // Scan for input until the user signals they're able to play, then update the state.
    if (snek->status == START_MENU) {
        // Render to the screen:
        snek_render_menu();

        // Poll for input.
        while (SDL_PollEvent(&snek->event) != 0) {
            if (snek->event.type == SDL_QUIT) {
                snek->status = QUIT_LOOP;
                return;
            }

            if (snek->event.type == SDL_KEYDOWN) {
                // Update snek direction and proceed to game.
                snek_input();
                snek->status = MID_GAME;

                // If a difficulty option key is pressed, update difficulty and don't progress to game
                // Scan again for more inputs
                switch (snek->event.key.keysym.sym) {
                    case SDLK_e:
                        snek->difficulty = EASY;
                        snek->status = START_MENU;
                        break;

                    case SDLK_r:
                        snek->difficulty = REGULAR;
                        snek->status = START_MENU;
                        break;

                    case SDLK_q:
                        snek->difficulty = HARD;
                        snek->status = START_MENU;
                        break;
                }
            }
            break;
        }
    }

    // Run the loop program procedure for the main gameplay:
    if (snek->status == MID_GAME) {
        //printf("I'm in the mid game\n");
        // Poll for input.
        while (SDL_PollEvent(&snek->event) != 0) {
            if (snek->event.type == SDL_QUIT) {
                snek->status = QUIT_LOOP;
                return;
            }

            if (snek->event.type == SDL_KEYDOWN) {
                if (snek->event.key.keysym.sym == SDLK_p) {
                    snek->status = PAUSE;
                    break;
                }
                snek_input();
            }
            break;
        }

        // Calculate time elapsed since last time this code was run.
        // If it has been long enough, then update the snek and render to the screen the updated gameplay.
        // Store time to calculate when to update entities and render world
        // The number the difficulty is set to is actually the milliseconds in delay it takes between updates!

        snek->current_time = SDL_GetTicks();
        if (snek->current_time > snek->last_time + snek->difficulty) {
            // Check to make sure the game is still won or not.
            // If not, set status to game over
            if (snek_update() == false) {
                snek->status = GAME_OVER;
            }

            // Render to the screen
            snek_render();

            // Set value for last time since this function called so we can compare it to current time to check how long has passed since then.
            snek->last_time = snek->current_time;
        }
    }

    // Run the loop program procedure for the game over screen.
    if (snek->status == GAME_OVER) {
        // Render game over screen.
        snek_render_game_over();

        // Poll for input
        while (SDL_PollEvent(&snek->event) != 0) {
            if (snek->event.type == SDL_QUIT) {
                snek->status = QUIT_LOOP;
                return;
            }

            // On any key press, reset the game and go back to the start menu.
            if (snek->event.type == SDL_KEYDOWN) {
                snek_entity_free(snek->head);
                snek->head = snek_entity_new(MAP_ROWS/2, MAP_COLUMNS/2);
                snek_food_entity_spawn();
                snek_map_init();
                snek->score = 1;
                snek->status = START_MENU;
            }
            break;
        }
    }

    if (snek->status == PAUSE) {
        // Scan for input and resume when any key is pressed.
        while (SDL_PollEvent(&snek->event) != 0) {
            if (snek->event.type == SDL_QUIT) {
                snek->status = QUIT_LOOP;
                return;
            }

            if (snek->event.type == SDL_KEYDOWN) {
                if (snek->event.key.keysym.sym == SDLK_p) {
                    snek->status = MID_GAME;
                    break;
                }
            }
            break;
        }
    }
    return;
}

int main(void) {
    // Initialise the snek program.
    if (snek_init() == false) {
        printf("main(): snek_init() function returned false. Returning.\n");
        return 0;
    }

    // Run the main program loop.
    // If the code is an emscripten/webassembly environment.
    #ifdef __EMSCRIPTEN__
        emscripten_set_main_loop(snek_loop, 0, 1);
    #endif

    // On a traditional OS, run the loop function inside a while loop.
    while (snek->status != QUIT_LOOP) {
        snek_loop();
    }
    
    snek_quit();
    return 0;
}
