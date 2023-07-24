// Enterprise by Ash Amin. (Copyright 2023)

// Third Party Disclosure: This program code is based on the code here: 
// https://github.com/Immediate-Mode-UI/Nuklear/blob/master/demo/sdl_opengl2/main.c
// It is licensed under the MIT.
// Big thank you to the Nuklear team for providing the GUI API.

/*
File description: main.c contains a program status struct and manages the 
execution of the enterprise software.

Data structures:
- The program struct stores information about rendering, program status, and the
enterprise database itself. 
*/

// Import C standard libraries.
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <limits.h>
#include <time.h>

// Import Nuklear.
#ifndef ENTERPRISE_NUKLEAR_LIBRARY_IMPORT
#define ENTERPRISE_NUKLEAR_LIBRARY_IMPORT
    #define NK_INCLUDE_FIXED_TYPES
    #define NK_INCLUDE_STANDARD_IO
    #define NK_INCLUDE_STANDARD_VARARGS
    #define NK_INCLUDE_DEFAULT_ALLOCATOR
    #define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
    #define NK_INCLUDE_FONT_BAKING
    #define NK_INCLUDE_DEFAULT_FONT
    #define NK_IMPLEMENTATION
    #define NK_SDL_GLES2_IMPLEMENTATION
    #include "../third_party/Nuklear/nuklear.h"
    #include "../third_party/Nuklear/demo/sdl_opengles2/nuklear_sdl_gles2.h"
    #include "../third_party/Nuklear/demo/common/style.c"
#endif

// Import enterprise.
#include "constants.c"
#include "enterprise.c"

#ifndef PROGRAM_STATES
#define PROGRAM_STATES
#include "program_states.c"
#endif

// Define the program state structure used to hold everything.

struct program {
    // GUI related structures.
    SDL_Window* window;
    struct nk_context *nk_context;
    SDL_GLContext glctx;

    // Program status.
    enum program_status status;
    
    // Enterprise data structure.
    struct enterprise* enterprise;
};

// Initialise a new program state and initialise associated libraries.
// Returns program state pointer on success, or NULL on failure.
struct program* program_init() {
    // Allocate program state memory and return pointer to it.
    struct program* program = malloc(sizeof(struct program));
    if (program == NULL) {printf("Failed to initailise program state.\n");
    return NULL;}

    // Initialise SDL.
    SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "0");
    SDL_GL_SetAttribute (SDL_GL_CONTEXT_FLAGS, 
    SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute (SDL_GL_CONTEXT_PROFILE_MASK, 
    SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    // Initialise SDL Window
    program->window = SDL_CreateWindow("Enterprise",
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL|
    SDL_WINDOW_SHOWN|SDL_WINDOW_ALLOW_HIGHDPI);

    // Return NULL and free resources on failure
    if (program->window == NULL) {
        printf("Failed to initialse SDL Window.SDL_GetError(): %s", 
        SDL_GetError());SDL_Quit();free(program);return NULL;
    };

    // Initialise OpenGL. Exit on failure
    program->glctx = SDL_GL_CreateContext(program->window);
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    if (program->glctx == NULL) {printf("Failed to initialise OpenGL context\
    . SDL_GetError %s\n", SDL_GetError());
    SDL_DestroyWindow(program->window);SDL_Quit();free(program);return NULL;}

    // Initialise Nuklear library context.
    // Exit on failure.
    program->nk_context = nk_sdl_init(program->window);
    if (program->nk_context == NULL) {printf("Failed to init Nuklear library");
    SDL_GL_DeleteContext(program->glctx);
    SDL_DestroyWindow(program->window);SDL_Quit();free(program);return NULL;}

    // Load font.
    struct nk_font_atlas *atlas;
    nk_sdl_font_stash_begin(&atlas);
    struct nk_font *font = nk_font_atlas_add_from_file(atlas, "ProggyClean.ttf", 
    ENTERPRISE_FONT_SIZE, 0);
    nk_sdl_font_stash_end();

    // Return NULL on failure to load font.
    if (font == NULL) {printf("Failed to load font 'ProggyClean.ttf'. Exiting.\n");
    nk_sdl_shutdown();SDL_GL_DeleteContext(program->glctx);
    SDL_DestroyWindow(program->window);SDL_Quit();free(program);return NULL;}

    // Set font on successful font load.
    nk_style_set_font(program->nk_context, &font->handle);

    // Set Nuklear theme:
    set_style(program->nk_context, THEME_BLUE);

    // Set program status:
    program->status = program_status_enterprise_menu;

    // Initialise Enterprise database.
    program->enterprise = enterprise_new();

    // Return program pointer.
    return program;
}

// Runs the main loop of the program
void program_loop(void* loop_argument) {
    // Load in the program state.
    struct program* program = (struct program*)loop_argument;

    // Handle SDL Input
    SDL_Event evt;
    nk_input_begin(program->nk_context);
    while (SDL_PollEvent(&evt)) {
        if (evt.type == SDL_QUIT) program->status = program_status_quit;
        nk_sdl_handle_event(&evt);
    }
    nk_input_end(program->nk_context);


    // Initialise and draw Nuklear GUI widgets + elements.
    // Switch between various menus depending on program state.
    if (nk_begin(program->nk_context, "Enterprise", 
    nk_rect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT),NK_WINDOW_BORDER)) {
        if (program->status == program_status_enterprise_menu) {
            program->status = enterprise_menu(program->nk_context\
            ,program->enterprise);
        }

        if (program->status == program_status_facility_table) {
            program->status = facility_table(program->nk_context\
            ,program->enterprise->facility_list);
        }

        if (program->status == program_status_facility_editor) {
            program->status = facility_editor(program->nk_context\
            ,program->enterprise->facility_list);
        }
        
        if (program->status == program_status_employee_table) {
            program->status = employee_table(program->nk_context\
            ,program->enterprise->employee_list);
        }

        if (program->status == program_status_employee_editor) {
            program->status = employee_editor(program->nk_context\
            ,program->enterprise->employee_list);
        }

        // Select the currently selected employee and edit what
        // facilities said employee works at.
        if (program->status == program_status_employee_facility_editor) {
            program->status = employee_facility_editor(program->nk_context\
            ,employee_list_get_node(program->enterprise->employee_list,\
            program->enterprise->employee_list->id_currently_selected)\
            ->employee_facility_list, program->enterprise->facility_list);
        }
        
        // Show what facilities the currently selected employee works at.
        if (program->status == program_status_employee_facility_table) {
            program->status = employee_facility_table(program->nk_context\
            ,employee_list_get_node(program->enterprise->employee_list,\
            program->enterprise->employee_list->id_currently_selected)\
            ->employee_facility_list, program->enterprise->facility_list);
        }

        if (program->status == program_status_item_table) {
            program->status = item_table(program->nk_context\
            ,program->enterprise->item_list);
        }

        if (program->status == program_status_item_editor) {
            program->status = item_editor(program->nk_context\
            ,program->enterprise->item_list);
        }

        if (program->status == program_status_item_facility_editor) {
            program->status = item_facility_editor(program->nk_context\
            ,item_list_get_node(program->enterprise->item_list,\
            program->enterprise->item_list->id_currently_selected)\
            ->item_facility_list, program->enterprise->facility_list);
        }
        
        // Show what facilities the currently selected item works at.
        if (program->status == program_status_item_facility_table) {
            program->status = item_facility_table(program->nk_context\
            ,item_list_get_node(program->enterprise->item_list,\
            program->enterprise->item_list->id_currently_selected)\
            ->item_facility_list, program->enterprise->facility_list);
        }

        if (program->status == program_status_customer_table) {
            program->status = customer_table(program->nk_context\
            ,program->enterprise->customer_list);
        }

        if (program->status == program_status_customer_editor) {
            program->status = customer_editor(program->nk_context\
            ,program->enterprise->customer_list);
        }

        if (program->status == program_status_supplier_editor) {
            program->status = supplier_editor(program->nk_context\
            ,program->enterprise->supplier_list);
        }

        if (program->status == program_status_supplier_table) {
            program->status = supplier_table(program->nk_context\
            ,program->enterprise->supplier_list);
        }

        if (program->status == program_status_expense_editor) {
            program->status = expense_editor(program->nk_context\
            ,program->enterprise->expense_list);
        }

        if (program->status == program_status_expense_table) {
            program->status = expense_table(program->nk_context\
            ,program->enterprise->expense_list);
        }

        if (program->status == program_status_order_table) {
            program->status = order_editor(program->nk_context\
            ,program->enterprise->order_list);
        }

        if (program->status == program_status_order_editor) {
            program->status = order_editor(program->nk_context\
            ,program->enterprise->order_list);
        }

        
    }
    nk_end(program->nk_context);

    // Render the GUI.
    float bg[4];
    int win_width, win_height;
    nk_color_fv(bg, nk_rgb(28,48,62));
    SDL_GetWindowSize(program->window, &win_width, &win_height);
    glViewport(0, 0, win_width, win_height);
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(bg[0], bg[1], bg[2], bg[3]);
    nk_sdl_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_MEMORY, MAX_ELEMENT_MEMORY);
    SDL_GL_SwapWindow(program->window);
}

// Free all resources and exit.
void program_quit(struct program* program) {
    if (program == NULL) return;

    // Shutdown Nuklear
    nk_sdl_shutdown();

    // Shutdown OpenGL and SDL.
    SDL_GL_DeleteContext(program->glctx);
    SDL_DestroyWindow(program->window);
    SDL_Quit();

    // Free enterprise database memory.
    if (program->enterprise != NULL) enterprise_quit(program->enterprise);

    // Free program heap memory.
    free(program);
}

int main(void) {
    // Initailise the program state and everything required.
    // Return to OS on failure to initialise.
    struct program* program = program_init();
    if (program == NULL) return -1;

    // Run the main program loop, 
    // differently based on whether program is native or on web.
    #if defined(__EMSCRIPTEN__)
        #include <emscripten.h>
        emscripten_set_main_loop_arg(program_loop, (void*)program, 0, 1);
    #else
        while (program->status != program_status_quit) {
            program_loop((void*)program);
        }
    #endif

    // Free all resources associated with program and exit.
    program_quit(program);
    return 0;
}

