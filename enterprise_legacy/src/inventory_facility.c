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

#ifndef ENTERPRISE_LIBRARIES
#define ENTERPRISE_LIBRARIES
#include "constants.c"
#include "facilities.c"
#include "inventory.c"
#include "customers.c"
#include "suppliers.c"
#include "expenses.c"
#include "orders.c"
#endif

/* How item_facilitys work.
item_facilitys are stored in a struct that contains a pointer to the head of a
linked list containing all the item_facilitys. The struct that stores the linked
list is directly added to the enterprise struct and stores metadata about the
list. All item_facilitys are assigned a unique ID, no two item_facilitys can have the
same ID, the item_facility_list structure keeps track of that.

Data structures:
item_facility_node: An individual item_facility.
item_facility_list: A structure holding important metadata about the item_facility
linked list.

item_facility list metadata:
item_facility_list->id_last_assigned: This is incremented by 1 every time a new
item_facility is added in order to ensure no two item_facilitys ever have the same ID.

item_facility_list->id_currently_selected: This is the ID that is selected in the
item_facility editor dialogue.
*/

struct item_facility_node {
    char id[ENTERPRISE_STRING_LENGTH];
    char facility_id[ENTERPRISE_STRING_LENGTH];
    char quantity[ENTERPRISE_STRING_LENGTH];

    struct item_facility_node* prev;
    struct item_facility_node* next;
};

// item_facility node constructor and initialiser.
// Returns item_facility node on success, or NULL on failure.
struct item_facility_node *item_facility_node_new() {
    struct item_facility_node* item_facility = malloc(sizeof(struct item_facility_node));
    if (item_facility == NULL) return NULL;
    strcpy(item_facility->id, "");
    strcpy(item_facility->facility_id, "");
    strcpy(item_facility->quantity, "");

    item_facility->prev = NULL;
    item_facility->next = NULL;

    return item_facility;
}

// item_facility list metadata structure.
struct item_facility_list {
    struct item_facility_node *head;
    char id_last_assigned[ENTERPRISE_STRING_LENGTH];
    char id_currently_selected[ENTERPRISE_STRING_LENGTH];
    bool deletion_requested;
    bool addition_requested;
};

// item_facility list constructor.
// Returns item_facility list on success, or NULL on failure.
struct item_facility_list* item_facility_list_new() {
    struct item_facility_list* item_facility_list = malloc(sizeof(struct item_facility_list));
    if (item_facility_list == NULL) return NULL;
    item_facility_list->head = NULL;
    strcpy(item_facility_list->id_last_assigned, "0");
    strcpy(item_facility_list->id_currently_selected, "0");
    item_facility_list->deletion_requested = false;
    item_facility_list->addition_requested = false;
    return item_facility_list;
}

// Free an entire item_facility linked list
void item_facility_node_linked_list_free(struct item_facility_node* item_facility) {
    if (item_facility == NULL) return;
    struct item_facility_node* next = NULL;
    while (item_facility != NULL) {
        next = item_facility->next;
        free(item_facility);
        item_facility = next;
    }
}

// Free all memory associated with a item_facility list.
void item_facility_list_free(struct item_facility_list* item_facility_list) {
    if (item_facility_list == NULL) return;

    if (item_facility_list->head != NULL) {
        item_facility_node_linked_list_free(item_facility_list->head);
    }

    free(item_facility_list);
    return;
}

// Append a new item_facility to a item_facility list.
void item_facility_list_append(struct item_facility_list* item_facility_list) {
    if (item_facility_list == NULL) return;

    // Increment unique ID by 1.
    char buffer[ENTERPRISE_STRING_LENGTH];
    sprintf(buffer, "%lld", atoll(item_facility_list->id_last_assigned) + 1);
    strcpy(item_facility_list->id_last_assigned, buffer);

    // Add new node to start of item_facility list if item_facility linked list does
    // not exist.
    if (item_facility_list->head == NULL) {
        item_facility_list->head = item_facility_node_new();
        strcpy(item_facility_list->head->id, item_facility_list->id_last_assigned);
        strcpy(item_facility_list->id_currently_selected,
        item_facility_list->id_last_assigned);
        return;
    }

    // Else add the new node to the end of the list.
    struct item_facility_node* item_facility = item_facility_list->head;
    while (item_facility->next != NULL) {
        item_facility = item_facility->next;
    }
    
    item_facility->next = item_facility_node_new();
    item_facility->next->prev = item_facility;

    strcpy(item_facility->next->id, item_facility_list->id_last_assigned);
    strcpy(item_facility_list->id_currently_selected,
    item_facility_list->id_last_assigned);

    return;
}

// Get the number of item_facility nodes in the item_facility list
int item_facility_list_get_num_item_facility_nodes(struct item_facility_list* item_facility_list) {
    if (item_facility_list == NULL) return 0;
    if (item_facility_list->head == NULL) return 0;

    struct item_facility_node* item_facility = item_facility_list->head;
    int item_facility_count = 0;
    while (item_facility != NULL) {
        item_facility_count++;
        item_facility = item_facility->next;
    }
    return item_facility_count;
}

// Return a pointer to a item_facility node according to ID.
// Returns NULL on failure.
struct item_facility_node *item_facility_list_get_node\
(struct item_facility_list *item_facility_list, char *id) {
    if (item_facility_list == NULL || id == NULL) return NULL;
    if (item_facility_list->head == NULL) return NULL;

    struct item_facility_node* item_facility = item_facility_list->head;
    while (item_facility != NULL) {
        if (strcmp(item_facility->id, id) == 0) return item_facility;
        item_facility = item_facility->next;
    }
    return NULL;
}

// Return a pointer to a item_facility node according to facility ID.
// Returns NULL on failure.
struct item_facility_node *item_facility_list_get_node_by_facility_id\
(struct item_facility_list *item_facility_list, char *id) {
    if (item_facility_list == NULL || id == NULL) return NULL;
    if (item_facility_list->head == NULL) return NULL;

    struct item_facility_node* item_facility = item_facility_list->head;
    while (item_facility != NULL) {
        if (strcmp(item_facility->facility_id, id) == 0) return item_facility;
        item_facility = item_facility->next;
    }
    return NULL;
}

// Searches for a item_facility by ID and deletes it
void item_facility_list_delete_node(struct item_facility_list *item_facility_list, char *id) {
    if (item_facility_list == NULL || id == NULL) return;
    if (item_facility_list->head == NULL) return;

    // Delete the head if it is the ID that is requested to be deleted.
    if (strcmp(id, item_facility_list->head->id) == 0) {
        if (item_facility_list->head->next == NULL) {
            free(item_facility_list->head);
            item_facility_list->head = NULL;
            return;
        }

        struct item_facility_node *next = item_facility_list->head->next;
        free(item_facility_list->head);
        item_facility_list->head = next;
        return;
    }

    // Else search for the item_facility to delete.
    struct item_facility_node* item_facility = item_facility_list->head;
    while (item_facility != NULL) {
        if (strcmp(item_facility->id, id) == 0) break;
        item_facility = item_facility->next;
    }
    if (item_facility == NULL) return;

    // Fix the pointers of the next and previous nodes to the item_facility to be
    // deleted in order to ensure that traversal still works.

    struct item_facility_node* prev = item_facility->prev;
    struct item_facility_node* next = item_facility->next;

    if (prev != NULL) {
        if (next == NULL) {
            prev->next = NULL;
            strcpy(item_facility_list->id_currently_selected, prev->id);
        }
        if (next != NULL) {
            prev->next = next;
            next->prev = prev;
            strcpy(item_facility_list->id_currently_selected, next->id);
        }
    }

    if (prev == NULL) {
        if (next != NULL) {
            next->prev = NULL;
            strcpy(item_facility_list->id_currently_selected, next->id);
        }
    }

    // Delete the item_facility
    free(item_facility);
    return;
}

// Select the previous node as the currently selected item.
void item_facility_list_select_previous_node(struct item_facility_list *item_facility_list) {
    if (item_facility_list == NULL) return;
    if (item_facility_list->head == NULL) return;

    struct item_facility_node* item_facility = item_facility_list->head;
    while (item_facility != NULL) {
        if (strcmp(item_facility->id, item_facility_list->id_currently_selected) == 0) {
            if (item_facility->prev != NULL) {
                strcpy(item_facility_list->id_currently_selected, item_facility->prev->id);
                return;
            }
            if (item_facility->prev == NULL) {
                struct item_facility_node* temp = item_facility;
                while (temp->next != NULL) {
                    temp = temp->next;
                }
                strcpy(item_facility_list->id_currently_selected, temp->id);
                return;
            }
        }
        item_facility = item_facility->next;
    }
}

// Select the next node as the currently selected item.
void item_facility_list_select_next_node(struct item_facility_list *item_facility_list) {
    if (item_facility_list == NULL) return;
    if (item_facility_list->head == NULL) return;

    struct item_facility_node* item_facility = item_facility_list->head;
    while (item_facility != NULL) {
        if (strcmp(item_facility->id, item_facility_list->id_currently_selected) == 0) {
            if (item_facility->next != NULL) {
                strcpy(item_facility_list->id_currently_selected, item_facility->next->id);
                return;
            }
            if (item_facility->next == NULL) {
                strcpy(item_facility_list->id_currently_selected,\
                item_facility_list->head->id);
                return;
            }
        }
        item_facility = item_facility->next;
    }
}

// Change the currently selected ID to the passed in ID in the item_facility list.
void item_facility_list_set_selected_id\
(struct item_facility_list *item_facility_list, char* id) {
    if (item_facility_list == NULL || id == NULL) return;
    if (item_facility_list->head == NULL) return;

    struct item_facility_node* item_facility = item_facility_list->head;
    while (item_facility != NULL) {
        if (strcmp(item_facility->id, id) == 0) {
            strcpy(item_facility_list->id_currently_selected, id);
        }
        item_facility = item_facility->next;
    }
}

// Render the item facilities table GUI.
// This is an overview of the facilities that an item works at.
enum program_status item_facility_table(struct nk_context* ctx,\
struct item_facility_list* item_facility_list, \
struct facility_list* facility_list) {
    if (ctx == NULL || item_facility_list == NULL)\
    return program_status_item_editor;

    // Button to return to item editor.
    nk_layout_row_dynamic(ctx, ENTERPRISE_WIDGET_HEIGHT, 1);
    if (nk_button_label(ctx, "Return to Item Editor")) {
        return program_status_item_editor;
    }

    // Handle case where there are no facilities.
    if (facility_list->head == NULL) {
        nk_label(ctx, "There are no facilities available to add item stock to.",\
        NK_TEXT_CENTERED);
        if (nk_button_label(ctx, "Go to Facility Table.")) {
            return program_status_facility_table;
        }
    }

    else {
        // Display the title.
        nk_label(ctx, "Item Facilities Menu", NK_TEXT_CENTERED);
        nk_label(ctx, "Item is in stock at the following facilities: ",\
         NK_TEXT_CENTERED);

        // Present all facilities that the item works for.
        struct item_facility_node* item_facility\
        = item_facility_list->head;
        char* print_buffer = malloc(sizeof(char) * ENTERPRISE_STRING_LENGTH * 10);
        while (item_facility != NULL) {

            struct facility_node* facility = facility_list_get_node(\
            facility_list, item_facility->id);

            if (facility == NULL) break;
 
            sprintf(print_buffer, "Facility ID: %s Name: %s", \
            item_facility->id, facility->name);

            if (nk_button_label(ctx, print_buffer)) {
                strcpy(item_facility_list->id_currently_selected,\
                item_facility->id);
                free(print_buffer);
                return program_status_item_facility_editor;
            }
            item_facility = item_facility->next;
        }
        free(print_buffer);

        // Create button to add new item facilities to the table.
        nk_layout_row_dynamic(ctx, ENTERPRISE_WIDGET_HEIGHT, 1);
        if (nk_button_label(ctx, "Add Facility")) {
            if (item_facility_list_get_num_item_facility_nodes\
            (item_facility_list) != facility_list_get_num_facility_nodes(\
            facility_list)) {
            item_facility_list->addition_requested = true;
        }
        }

        /* If we want to add a new facility.
        Search through all the facilities in the facility table, display the 
        facilities that aren't currently worked for by the item, and 
        if clicked on, add it to the list of facilities the current item 
        works for. */
        if (item_facility_list->addition_requested == true) {
            print_buffer = malloc(sizeof(char) * ENTERPRISE_STRING_LENGTH * 10);
            struct facility_node* facility = facility_list->head;

            while (facility != NULL) {
                bool show = false;
                sprintf(print_buffer, "ID : %s Facility Name: %s",\
                facility->id, facility->name);
                if (item_facility_list_get_node_by_facility_id(\
                item_facility_list,facility->id) == NULL) show = true;
                    if (show == true) {
                        if (nk_button_label(ctx, print_buffer)) {
                            item_facility_list_append(item_facility_list);

                            strcpy(item_facility_list_get_node\
                            (item_facility_list,\
                            item_facility_list->id_currently_selected)\
                            ->facility_id, facility->id);
                            item_facility_list->addition_requested = false;
                        }
                    }
                facility = facility->next;
            }
            free(print_buffer);
        }
    }
    return program_status_item_facility_table;
}

// Render the item_facility editor GUI.
// It gives the user an opportunity to edit a currently selected item_facility.
enum program_status item_facility_editor(struct nk_context* ctx,\
struct item_facility_list* item_facility_list, \
struct facility_list* facility_list) {
    if (ctx == NULL || item_facility_list == NULL)
        return program_status_enterprise_menu;

    // Display title and return to item_facility table button.
    nk_layout_row_dynamic(ctx, ENTERPRISE_WIDGET_HEIGHT, 1);
    if (nk_button_label(ctx, "Return to Item Facility Stock Table")) {
        return program_status_item_facility_table;
    }
    nk_label(ctx, "Item Facility Stock Editor", NK_TEXT_CENTERED);
    
    // Select currently selected item_facility.
    struct item_facility_node* item_facility = \
    item_facility_list_get_node(item_facility_list,\
    item_facility_list->id_currently_selected);

    // If the currently selected item_facility does not exist:
    if (item_facility == NULL) {
        // Select the first item_facility instead.
        if (item_facility_list->head != NULL) {
            strcpy(item_facility_list->id_currently_selected, \
            item_facility_list->head->id);
            return program_status_item_facility_editor;
        }

        // Else tell the user to create a new item_facility 
        // if the first item_facility doesn't exist.
        else {
            nk_label(ctx, "No facilities found.", NK_TEXT_CENTERED);
            if (nk_button_label(ctx, "New Facility")) {
                item_facility_list->addition_requested = true;

            }
            return program_status_item_facility_editor;
        }
    }

    // Display edit fields to edit item_facility entries.
    nk_layout_row_template_begin(ctx, ENTERPRISE_WIDGET_HEIGHT);
    nk_layout_row_template_push_static(ctx, 150);
    nk_layout_row_template_push_dynamic(ctx);
    nk_layout_row_template_end(ctx);

    nk_label(ctx, "Facility ID: ", NK_TEXT_LEFT);
    nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, \
    item_facility->facility_id, ENTERPRISE_STRING_LENGTH, nk_filter_default);

    nk_label(ctx, "Quantity: ", NK_TEXT_LEFT);
    nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, \
    item_facility->quantity, ENTERPRISE_STRING_LENGTH, nk_filter_default);

    // Move between next and previous item_facilitys.
    nk_layout_row_dynamic(ctx, ENTERPRISE_WIDGET_HEIGHT, 2);
    if (nk_button_symbol_label\
    (ctx, NK_SYMBOL_TRIANGLE_LEFT, "prev", NK_TEXT_RIGHT)) {
        item_facility_list_select_previous_node(item_facility_list);
    }

    if (nk_button_symbol_label\
    (ctx, NK_SYMBOL_TRIANGLE_RIGHT, "next", NK_TEXT_LEFT)) {
        item_facility_list_select_next_node(item_facility_list);
    }

    nk_layout_row_dynamic(ctx, ENTERPRISE_WIDGET_HEIGHT, 1);

    // Create deletion button.
    // This will ask the user to confirm whether they want to delete the node.
    if (nk_button_label(ctx, "Delete Facility")) {
        item_facility_list->deletion_requested = true;
    }

    if (item_facility_list->deletion_requested == true) {
        nk_label(ctx, "Confirm deletion?", NK_TEXT_CENTERED);
        nk_layout_row_dynamic(ctx, ENTERPRISE_WIDGET_HEIGHT, 2);

        if (nk_button_label(ctx, "Yes")) {
            item_facility_list_delete_node(item_facility_list, item_facility->id);
            item_facility_list->deletion_requested = false;
        }
        if (nk_button_label(ctx, "No")) {
            item_facility_list->deletion_requested = false;
        }        
    }

     // Create button to add new item facilities to the table.
    if (nk_button_label(ctx, "Add Facility")) {
        if (item_facility_list_get_num_item_facility_nodes\
        (item_facility_list) != facility_list_get_num_facility_nodes(\
        facility_list)) {
            item_facility_list->addition_requested = true;
        }
    }

    if (item_facility_list->addition_requested == true) {
            char* print_buffer = malloc(sizeof(char) * ENTERPRISE_STRING_LENGTH * 10);
            struct facility_node* facility = facility_list->head;

            while (facility != NULL) {
                bool show = false;
                sprintf(print_buffer, "ID : %s Facility Name: %s",\
                facility->id, facility->name);
                if (item_facility_list_get_node_by_facility_id(\
                item_facility_list,facility->id) == NULL) show = true;
                    if (show == true) {
                        if (nk_button_label(ctx, print_buffer)) {
                            item_facility_list_append(item_facility_list);

                            strcpy(item_facility_list_get_node\
                            (item_facility_list,\
                            item_facility_list->id_currently_selected)\
                            ->facility_id, facility->id);
                            item_facility_list->addition_requested = false;
                        }
                    }
                facility = facility->next;
            }
            free(print_buffer);
            
        }
    
    return program_status_item_facility_editor;
}