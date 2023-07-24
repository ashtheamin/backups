#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <limits.h>
#include <time.h>

#include "constants.c"

#ifndef PROGRAM_STATES
#define PROGRAM_STATES
#include "program_states.c"
#endif


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


/* How facilities work.
Facilities are stored in a struct that contains a pointer to the head of a
linked list containing all the facilities. The struct that stores the linked
list is directly added to the enterprise struct and stores metadata about the
list. All facilities are assigned a unique ID, no two facilities can have the
same ID, the facility_list structure keeps track of that.

Data structures:
Facility_node: An individual facility.
Facility_list: A structure holding important metadata about the facility
linked list.

Facility list metadata:
facility_list->id_last_assigned: This is incremented by 1 every time a new
facility is added in order to ensure no two facilities ever have the same ID.

facility_list->id_currently_selected: This is the ID that is selected in the
facility editor dialogue.
*/

// Facility node.
enum facility_type \
{facility_type_office, facility_type_store, facility_type_warehouse};

struct facility_node {
    char id[ENTERPRISE_STRING_LENGTH];
    char name[ENTERPRISE_STRING_LENGTH];
    char email[ENTERPRISE_STRING_LENGTH];
    char phone[ENTERPRISE_STRING_LENGTH];
    char address[ENTERPRISE_STRING_LENGTH];

    enum facility_type type;

    struct facility_node* prev;
    struct facility_node* next;
};

// Facility node constructor and initialiser.
// Returns facility node on success, or NULL on failure.
struct facility_node *facility_node_new() {
    struct facility_node* facility = malloc(sizeof(struct facility_node));
    if (facility == NULL) return NULL;
    strcpy(facility->id, "");
    strcpy(facility->name, "");
    strcpy(facility->email, "");
    strcpy(facility->phone, "");
    strcpy(facility->address, "");
    
    facility->type = facility_type_office;

    facility->prev = NULL;
    facility->next = NULL;

    return facility;
}

// Facility list metadata structure.
struct facility_list {
    struct facility_node *head;
    char id_last_assigned[ENTERPRISE_STRING_LENGTH];
    char id_currently_selected[ENTERPRISE_STRING_LENGTH];
    bool deletion_requested;
};

// Facility list constructor.
// Returns facility list on success, or NULL on failure.
struct facility_list* facility_list_new() {
    struct facility_list* facility_list = malloc(sizeof(struct facility_list));
    if (facility_list == NULL) return NULL;
    facility_list->head = NULL;
    strcpy(facility_list->id_last_assigned, "0");
    strcpy(facility_list->id_currently_selected, "0");
    facility_list->deletion_requested = false;
    return facility_list;
}

// Free an entire facility linked list
void facility_node_linked_list_free(struct facility_node* facility) {
    if (facility == NULL) return;
    struct facility_node* next = NULL;
    while (facility != NULL) {
        next = facility->next;
        free(facility);
        facility = next;
    }
}

// Free all memory associated with a facility list.
void facility_list_free(struct facility_list* facility_list) {
    if (facility_list == NULL) return;

    if (facility_list->head != NULL) {
        facility_node_linked_list_free(facility_list->head);
    }

    free(facility_list);
    return;
}

// Append a new facility to a facility list.
void facility_list_append(struct facility_list* facility_list) {
    if (facility_list == NULL) return;

    // Increment unique ID by 1.
    char buffer[ENTERPRISE_STRING_LENGTH];
    sprintf(buffer, "%lld", atoll(facility_list->id_last_assigned) + 1);
    strcpy(facility_list->id_last_assigned, buffer);

    // Add new node to start of facility list if facility linked list does
    // not exist.
    if (facility_list->head == NULL) {
        facility_list->head = facility_node_new();
        strcpy(facility_list->head->id, facility_list->id_last_assigned);
        strcpy(facility_list->id_currently_selected,
        facility_list->id_last_assigned);
        return;
    }

    // Else add the new node to the end of the list.
    struct facility_node* facility = facility_list->head;
    while (facility->next != NULL) {
        facility = facility->next;
    }
    
    facility->next = facility_node_new();
    facility->next->prev = facility;

    strcpy(facility->next->id, facility_list->id_last_assigned);
    strcpy(facility_list->id_currently_selected,
    facility_list->id_last_assigned);

    return;
}

// Get the number of facility nodes in the facility list
int facility_list_get_num_facility_nodes(struct facility_list* facility_list) {
    if (facility_list == NULL) return 0;
    if (facility_list->head == NULL) return 0;

    struct facility_node* facility = facility_list->head;
    int facility_count = 0;
    while (facility != NULL) {
        facility_count++;
        facility = facility->next;
    }
    return facility_count;
}

// Return a pointer to a facility node according to ID.
// Returns NULL on failure.
struct facility_node *facility_list_get_node\
(struct facility_list *facility_list, char *id) {
    if (facility_list == NULL || id == NULL) return NULL;
    if (facility_list->head == NULL) return NULL;

    struct facility_node* facility = facility_list->head;
    while (facility != NULL) {
        if (strcmp(facility->id, id) == 0) return facility;
        facility = facility->next;
    }
    return NULL;
}

// Searches for a facility by ID and deletes it
void facility_list_delete_node(struct facility_list *facility_list, char *id) {
    if (facility_list == NULL || id == NULL) return;
    if (facility_list->head == NULL) return;

    // Delete the head if it is the ID that is requested to be deleted.
    if (strcmp(id, facility_list->head->id) == 0) {
        if (facility_list->head->next == NULL) {
            free(facility_list->head);
            facility_list->head = NULL;
            return;
        }

        struct facility_node *next = facility_list->head->next;
        free(facility_list->head);
        facility_list->head = next;
        return;
    }

    // Else search for the facility to delete.
    struct facility_node* facility = facility_list->head;
    while (facility != NULL) {
        if (strcmp(facility->id, id) == 0) break;
        facility = facility->next;
    }
    if (facility == NULL) return;

    // Fix the pointers of the next and previous nodes to the facility to be
    // deleted in order to ensure that traversal still works.

    struct facility_node* prev = facility->prev;
    struct facility_node* next = facility->next;

    if (prev != NULL) {
        if (next == NULL) {
            prev->next = NULL;
            strcpy(facility_list->id_currently_selected, prev->id);
        }
        if (next != NULL) {
            prev->next = next;
            next->prev = prev;
            strcpy(facility_list->id_currently_selected, next->id);
        }
    }

    if (prev == NULL) {
        if (next != NULL) {
            next->prev = NULL;
            strcpy(facility_list->id_currently_selected, next->id);
        }
    }

    // Delete the facility
    free(facility);
    return;
}

// Select the previous node as the currently selected item.
void facility_list_select_previous_node(struct facility_list *facility_list) {
    if (facility_list == NULL) return;
    if (facility_list->head == NULL) return;

    struct facility_node* facility = facility_list->head;
    while (facility != NULL) {
        if (strcmp(facility->id, facility_list->id_currently_selected) == 0) {
            if (facility->prev != NULL) {
                strcpy(facility_list->id_currently_selected, facility->prev->id);
                return;
            }
            if (facility->prev == NULL) {
                struct facility_node* temp = facility;
                while (temp->next != NULL) {
                    temp = temp->next;
                }
                strcpy(facility_list->id_currently_selected, temp->id);
                return;
            }
        }
        facility = facility->next;
    }
}

// Select the next node as the currently selected item.
void facility_list_select_next_node(struct facility_list *facility_list) {
    if (facility_list == NULL) return;
    if (facility_list->head == NULL) return;

    struct facility_node* facility = facility_list->head;
    while (facility != NULL) {
        if (strcmp(facility->id, facility_list->id_currently_selected) == 0) {
            if (facility->next != NULL) {
                strcpy(facility_list->id_currently_selected, facility->next->id);
                return;
            }
            if (facility->next == NULL) {
                strcpy(facility_list->id_currently_selected,\
                facility_list->head->id);
                return;
            }
        }
        facility = facility->next;
    }
}

// Change the currently selected ID to the passed in ID in the facility list.
void facility_list_set_selected_id\
(struct facility_list *facility_list, char* id) {
    if (facility_list == NULL || id == NULL) return;
    if (facility_list->head == NULL) return;

    struct facility_node* facility = facility_list->head;
    while (facility != NULL) {
        if (strcmp(facility->id, id) == 0) {
            strcpy(facility_list->id_currently_selected, id);
        }
        facility = facility->next;
    }
}

// Get the facility type back as a string:
char* facility_list_get_node_type(struct facility_list* facility_list, char* id) {
    if (facility_list == NULL || id == NULL) return NULL;
    
    struct facility_node* facility = facility_list_get_node(facility_list, id);
    if (facility == NULL) return NULL;

    if (facility->type == facility_type_office) return "Office";
    if (facility->type == facility_type_store) return "Store";
    if (facility->type == facility_type_warehouse) return "Warehouse";

    return NULL;
}

// Render the facility table GUI.
// This function displays a list of facilities as a table that can be selected.
// It is an overview.
enum program_status facility_table(struct nk_context* ctx,\
struct facility_list* facility_list) {
    if (ctx == NULL || facility_list == NULL) return program_status_running;

    // Button to return to enterprise menu.
    nk_layout_row_dynamic(ctx, ENTERPRISE_WIDGET_HEIGHT, 1);
    if (nk_button_label(ctx, "Return to Enterprise Menu")) {
        return program_status_enterprise_menu;
    }

    // Display the title.
    nk_label(ctx, "Facility Menu", NK_TEXT_CENTERED);

    // Create button to add new facilities to the table.
    if (nk_button_label(ctx, "New Facility")) {
        facility_list_append(facility_list);
    }

    // If there are no facilities, warn the user.
    if (facility_list->head == NULL) {
        nk_label(ctx, "No facilities found.", NK_TEXT_CENTERED);
        return program_status_facility_table;
    }

    /* If there are facilities, go through the linked list of facilities.
    Copy the data from each facility and make it a button label.
    When the button is pressed, set the currently selected facility to that
    facility and switch to facility editor.*/
    struct facility_node* facility = facility_list->head;
    char* print_buffer = malloc(sizeof(char) * ENTERPRISE_STRING_LENGTH * 10);
    while (facility != NULL) {
        sprintf(print_buffer, \
        "ID: %s Type: %s Name: %s Email: %s Phone: %s Address: %s",facility->id,\
        facility_list_get_node_type(facility_list, facility->id),
        facility->name, facility->email, facility->phone, facility->address);

        if (nk_button_label(ctx, print_buffer)) {
            strcpy(facility_list->id_currently_selected, facility->id);
            free(print_buffer);
            return program_status_facility_editor;
        }
        facility = facility->next;
    }
    free(print_buffer);
    return program_status_facility_table;
}

// Render the facility editor GUI.
// It gives the user an opportunity to edit a currently selected facility.
enum program_status facility_editor(struct nk_context* ctx,\
struct facility_list* facility_list) {
    if (ctx == NULL || facility_list == NULL)
        return program_status_enterprise_menu;

    // Display title and return to facility table button.
    nk_layout_row_dynamic(ctx, ENTERPRISE_WIDGET_HEIGHT, 1);
    if (nk_button_label(ctx, "Return to Facility Table")) {
        return program_status_facility_table;
    }
    nk_label(ctx, "Facility Editor", NK_TEXT_CENTERED);
    
    // Select currently selected facility.
    struct facility_node* facility = facility_list_get_node\
    (facility_list, facility_list->id_currently_selected);

    // If the currently selected facility does not exist:
    if (facility == NULL) {
        // Select the first facility instead.
        if (facility_list->head != NULL) {
            strcpy(facility_list->id_currently_selected, facility_list->head->id);
            return program_status_facility_editor;
        }

        // Else tell the user to create a new facility 
        // if the first facility doesn't exist.
        else {
            nk_label(ctx, "No Facilities found.", NK_TEXT_CENTERED);
            if (nk_button_label(ctx, "New Facility")) {
                facility_list_append(facility_list);
            }
            return program_status_facility_editor;
        }
    }

    // Display edit fields to edit facility entries.
    nk_layout_row_template_begin(ctx, ENTERPRISE_WIDGET_HEIGHT);
    nk_layout_row_template_push_static(ctx, 150);
    nk_layout_row_template_push_dynamic(ctx);
    nk_layout_row_template_end(ctx);

    // Create combo box for facility type.
    int type = 0;
    if (facility->type == facility_type_office) type = 0;
    if (facility->type == facility_type_store) type = 1;
    if (facility->type == facility_type_warehouse) type = 2;

    const char* facility_types[] = {"Office", "Store", "Warehouse"};
    nk_label(ctx, "Type: ", NK_TEXT_LEFT);
    type = nk_combo(ctx, facility_types, NK_LEN(facility_types), type, \
    ENTERPRISE_WIDGET_HEIGHT, nk_vec2(WINDOW_WIDTH, 200));

    if (type == 0) facility->type = facility_type_office;
    if (type == 1) facility->type = facility_type_store;
    if (type == 2) facility->type = facility_type_warehouse;

    nk_label(ctx, "ID: ", NK_TEXT_LEFT);
    nk_edit_string_zero_terminated(ctx, NK_EDIT_READ_ONLY, \
    facility->id, ENTERPRISE_STRING_LENGTH, nk_filter_default);

    nk_label(ctx, "Name: ", NK_TEXT_LEFT);
    nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, \
    facility->name, ENTERPRISE_STRING_LENGTH, nk_filter_default);

    nk_label(ctx, "Phone: ", NK_TEXT_LEFT);
    nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, \
    facility->phone, ENTERPRISE_STRING_LENGTH, nk_filter_default);

    nk_label(ctx, "Email: ", NK_TEXT_LEFT);
    nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, \
    facility->email, ENTERPRISE_STRING_LENGTH, nk_filter_default);

    nk_label(ctx, "Address: ", NK_TEXT_LEFT);
    nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, \
    facility->address, ENTERPRISE_STRING_LENGTH, nk_filter_default);

    // Move between next and previous facilities.
    nk_layout_row_dynamic(ctx, ENTERPRISE_WIDGET_HEIGHT, 2);
    if (nk_button_symbol_label\
    (ctx, NK_SYMBOL_TRIANGLE_LEFT, "prev", NK_TEXT_RIGHT)) {
        facility_list_select_previous_node(facility_list);
    }

    if (nk_button_symbol_label\
    (ctx, NK_SYMBOL_TRIANGLE_RIGHT, "next", NK_TEXT_LEFT)) {
        facility_list_select_next_node(facility_list);
    }

    nk_layout_row_dynamic(ctx, ENTERPRISE_WIDGET_HEIGHT, 1);
    // Create new facility button.
    if (nk_button_label(ctx, "New Facility")) {
        facility_list_append(facility_list);
    }

    // Create deletion button.
    // This will ask the user to confirm whether they want to delete the node.
    if (nk_button_label(ctx, "Delete Facility")) {
        facility_list->deletion_requested = true;
    }

    if (facility_list->deletion_requested == true) {
        nk_label(ctx, "Confirm deletion?", NK_TEXT_CENTERED);
        nk_layout_row_dynamic(ctx, ENTERPRISE_WIDGET_HEIGHT, 2);

        if (nk_button_label(ctx, "Yes")) {
            facility_list_delete_node(facility_list, facility->id);
            facility_list->deletion_requested = false;
        }
        if (nk_button_label(ctx, "No")) {
            facility_list->deletion_requested = false;
        }        
    }
    
    return program_status_facility_editor;
}