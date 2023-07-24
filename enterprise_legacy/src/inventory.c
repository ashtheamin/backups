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

#include "inventory_facility.c"


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

/* How items work.
items are stored in a struct that contains a pointer to the head of a
linked list containing all the items. The struct that stores the linked
list is directly added to the enterprise struct and stores metadata about the
list. All items are assigned a unique ID, no two items can have the
same ID, the item_list structure keeps track of that.

Data structures:
item_node: An individual item.
item_list: A structure holding important metadata about the item
linked list.

item list metadata:
item_list->id_last_assigned: This is incremented by 1 every time a new
item is added in order to ensure no two items ever have the same ID.

item_list->id_currently_selected: This is the ID that is selected in the
item editor dialogue.
*/

struct item_node {
    char id[ENTERPRISE_STRING_LENGTH];
    char name[ENTERPRISE_STRING_LENGTH];
    char retail_price[ENTERPRISE_STRING_LENGTH];
    char internal_cost[ENTERPRISE_STRING_LENGTH];

    struct item_facility_list* item_facility_list;

    struct item_node* prev;
    struct item_node* next;
};

// item node constructor and initialiser.
// Returns item node on success, or NULL on failure.
struct item_node *item_node_new() {
    struct item_node* item = malloc(sizeof(struct item_node));
    if (item == NULL) return NULL;
    strcpy(item->id, "");
    strcpy(item->name, "");
    strcpy(item->retail_price, "");
    strcpy(item->internal_cost, "");

    item->item_facility_list = NULL;

    item->prev = NULL;
    item->next = NULL;

    return item;
}

// item list metadata structure.
struct item_list {
    struct item_node *head;
    char id_last_assigned[ENTERPRISE_STRING_LENGTH];
    char id_currently_selected[ENTERPRISE_STRING_LENGTH];
    bool deletion_requested;
};

// item list constructor.
// Returns item list on success, or NULL on failure.
struct item_list* item_list_new() {
    struct item_list* item_list = malloc(sizeof(struct item_list));
    if (item_list == NULL) return NULL;
    item_list->head = NULL;
    strcpy(item_list->id_last_assigned, "0");
    strcpy(item_list->id_currently_selected, "0");
    item_list->deletion_requested = false;
    return item_list;
}

void item_node_free(struct item_node* item) {
    if (item == NULL) return;
    if (item->item_facility_list != NULL) {
        item_facility_list_free(item->item_facility_list);
    }
    free(item);
}

// Free an entire item linked list
void item_node_linked_list_free(struct item_node* item) {
    if (item == NULL) return;
    struct item_node* next = NULL;
    while (item != NULL) {
        next = item->next;
        item_node_free(item);
        item = next;
    }
}

// Free all memory associated with a item list.
void item_list_free(struct item_list* item_list) {
    if (item_list == NULL) return;

    if (item_list->head != NULL) {
        item_node_linked_list_free(item_list->head);
    }

    free(item_list);
    return;
}

// Append a new item to a item list.
void item_list_append(struct item_list* item_list) {
    if (item_list == NULL) return;

    // Increment unique ID by 1.
    char buffer[ENTERPRISE_STRING_LENGTH];
    sprintf(buffer, "%lld", atoll(item_list->id_last_assigned) + 1);
    strcpy(item_list->id_last_assigned, buffer);

    // Add new node to start of item list if item linked list does
    // not exist.
    if (item_list->head == NULL) {
        item_list->head = item_node_new();
        strcpy(item_list->head->id, item_list->id_last_assigned);
        strcpy(item_list->id_currently_selected,
        item_list->id_last_assigned);
        return;
    }

    // Else add the new node to the end of the list.
    struct item_node* item = item_list->head;
    while (item->next != NULL) {
        item = item->next;
    }
    
    item->next = item_node_new();
    item->next->prev = item;

    strcpy(item->next->id, item_list->id_last_assigned);
    strcpy(item_list->id_currently_selected,
    item_list->id_last_assigned);

    return;
}

// Get the number of item nodes in the item list
int item_list_get_num_item_nodes(struct item_list* item_list) {
    if (item_list == NULL) return 0;
    if (item_list->head == NULL) return 0;

    struct item_node* item = item_list->head;
    int item_count = 0;
    while (item != NULL) {
        item_count++;
        item = item->next;
    }
    return item_count;
}

// Return a pointer to a item node according to ID.
// Returns NULL on failure.
struct item_node *item_list_get_node\
(struct item_list *item_list, char *id) {
    if (item_list == NULL || id == NULL) return NULL;
    if (item_list->head == NULL) return NULL;

    struct item_node* item = item_list->head;
    while (item != NULL) {
        if (strcmp(item->id, id) == 0) return item;
        item = item->next;
    }
    return NULL;
}

// Searches for a item by ID and deletes it
void item_list_delete_node(struct item_list *item_list, char *id) {
    if (item_list == NULL || id == NULL) return;
    if (item_list->head == NULL) return;

    // Delete the head if it is the ID that is requested to be deleted.
    if (strcmp(id, item_list->head->id) == 0) {
        if (item_list->head->next == NULL) {
            item_node_free(item_list->head);
            item_list->head = NULL;
            return;
        }

        struct item_node *next = item_list->head->next;
        item_node_free(item_list->head);
        item_list->head = next;
        return;
    }

    // Else search for the item to delete.
    struct item_node* item = item_list->head;
    while (item != NULL) {
        if (strcmp(item->id, id) == 0) break;
        item = item->next;
    }
    if (item == NULL) return;

    // Fix the pointers of the next and previous nodes to the item to be
    // deleted in order to ensure that traversal still works.

    struct item_node* prev = item->prev;
    struct item_node* next = item->next;

    if (prev != NULL) {
        if (next == NULL) {
            prev->next = NULL;
            strcpy(item_list->id_currently_selected, prev->id);
        }
        if (next != NULL) {
            prev->next = next;
            next->prev = prev;
            strcpy(item_list->id_currently_selected, next->id);
        }
    }

    if (prev == NULL) {
        if (next != NULL) {
            next->prev = NULL;
            strcpy(item_list->id_currently_selected, next->id);
        }
    }

    // Delete the item
    item_node_free(item);
    return;
}

// Select the previous node as the currently selected item.
void item_list_select_previous_node(struct item_list *item_list) {
    if (item_list == NULL) return;
    if (item_list->head == NULL) return;

    struct item_node* item = item_list->head;
    while (item != NULL) {
        if (strcmp(item->id, item_list->id_currently_selected) == 0) {
            if (item->prev != NULL) {
                strcpy(item_list->id_currently_selected, item->prev->id);
                return;
            }
            if (item->prev == NULL) {
                struct item_node* temp = item;
                while (temp->next != NULL) {
                    temp = temp->next;
                }
                strcpy(item_list->id_currently_selected, temp->id);
                return;
            }
        }
        item = item->next;
    }
}

// Select the next node as the currently selected item.
void item_list_select_next_node(struct item_list *item_list) {
    if (item_list == NULL) return;
    if (item_list->head == NULL) return;

    struct item_node* item = item_list->head;
    while (item != NULL) {
        if (strcmp(item->id, item_list->id_currently_selected) == 0) {
            if (item->next != NULL) {
                strcpy(item_list->id_currently_selected, item->next->id);
                return;
            }
            if (item->next == NULL) {
                strcpy(item_list->id_currently_selected,\
                item_list->head->id);
                return;
            }
        }
        item = item->next;
    }
}

// Change the currently selected ID to the passed in ID in the item list.
void item_list_set_selected_id\
(struct item_list *item_list, char* id) {
    if (item_list == NULL || id == NULL) return;
    if (item_list->head == NULL) return;

    struct item_node* item = item_list->head;
    while (item != NULL) {
        if (strcmp(item->id, id) == 0) {
            strcpy(item_list->id_currently_selected, id);
        }
        item = item->next;
    }
}

// Render the item table GUI.
// This function displays a list of items as a table that can be selected.
// It is an overview.
enum program_status item_table(struct nk_context* ctx,\
struct item_list* item_list) {
    if (ctx == NULL || item_list == NULL) return program_status_running;

    // Button to return to enterprise menu.
    nk_layout_row_dynamic(ctx, ENTERPRISE_WIDGET_HEIGHT, 1);
    if (nk_button_label(ctx, "Return to Enterprise Menu")) {
        return program_status_enterprise_menu;
    }

    // Display the title.
    nk_label(ctx, "Inventory Menu", NK_TEXT_CENTERED);

    // Create button to add new items to the table.
    if (nk_button_label(ctx, "New Item")) {
        item_list_append(item_list);
    }

    // If there are no items, warn the user.
    if (item_list->head == NULL) {
        nk_label(ctx, "No Items found.", NK_TEXT_CENTERED);
        return program_status_item_table;
    }

    /* If there are items, go through the linked list of items.
    Copy the data from each item and make it a button label.
    When the button is pressed, set the currently selected item to that
    item and switch to item editor.*/
    struct item_node* item = item_list->head;
    char* print_buffer = malloc(sizeof(char) * ENTERPRISE_STRING_LENGTH * 10);
    while (item != NULL) {
        sprintf(print_buffer, \
        "ID: %s Name: %s Retail Price: %s Internal Cost: %s",item->id,\
        item->name, item->retail_price, item->internal_cost);

        if (nk_button_label(ctx, print_buffer)) {
            strcpy(item_list->id_currently_selected, item->id);
            free(print_buffer);
            return program_status_item_editor;
        }
        item = item->next;
    }
    free(print_buffer);
    return program_status_item_table;
}

// Render the item editor GUI.
// It gives the user an opportunity to edit a currently selected item.
enum program_status item_editor(struct nk_context* ctx,\
struct item_list* item_list) {
    if (ctx == NULL || item_list == NULL)
        return program_status_enterprise_menu;

    // Display title and return to inventory table button.
    nk_layout_row_dynamic(ctx, ENTERPRISE_WIDGET_HEIGHT, 1);
    if (nk_button_label(ctx, "Return to Inventory Table")) {
        return program_status_item_table;
    }
    nk_label(ctx, "Inventory Item Editor", NK_TEXT_CENTERED);
    
    // Select currently selected item.
    struct item_node* item = item_list_get_node\
    (item_list, item_list->id_currently_selected);

    // If the currently selected item does not exist:
    if (item == NULL) {
        // Select the first item instead.
        if (item_list->head != NULL) {
            strcpy(item_list->id_currently_selected, item_list->head->id);
            return program_status_item_editor;
        }

        // Else tell the user to create a new item 
        // if the first item doesn't exist.
        else {
            nk_label(ctx, "No Items found.", NK_TEXT_CENTERED);
            if (nk_button_label(ctx, "New Item")) {
                item_list_append(item_list);
            }
            return program_status_item_editor;
        }
    }

    // Display edit fields to edit item entries.
    nk_layout_row_template_begin(ctx, ENTERPRISE_WIDGET_HEIGHT);
    nk_layout_row_template_push_static(ctx, 200);
    nk_layout_row_template_push_dynamic(ctx);
    nk_layout_row_template_end(ctx);

    nk_label(ctx, "ID: ", NK_TEXT_LEFT);
    nk_edit_string_zero_terminated(ctx, NK_EDIT_READ_ONLY, \
    item->id, ENTERPRISE_STRING_LENGTH, nk_filter_default);

    nk_label(ctx, "Name: ", NK_TEXT_LEFT);
    nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, \
    item->name, ENTERPRISE_STRING_LENGTH, nk_filter_default);

    nk_label(ctx, "Retail Price: ", NK_TEXT_LEFT);
    nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, \
    item->retail_price, ENTERPRISE_STRING_LENGTH, nk_filter_default);

    nk_label(ctx, "Internal Cost: ", NK_TEXT_LEFT);
    nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, \
    item->internal_cost, ENTERPRISE_STRING_LENGTH, nk_filter_default);

    nk_layout_row_dynamic(ctx, ENTERPRISE_WIDGET_HEIGHT, 1);
    if (nk_button_label(ctx, "Stock")) {
        if (item->item_facility_list == NULL) {
            item->item_facility_list = item_facility_list_new();
        }
        return program_status_item_facility_table;
    }

    // Move between next and previous items.
    nk_layout_row_dynamic(ctx, ENTERPRISE_WIDGET_HEIGHT, 2);
    if (nk_button_symbol_label\
    (ctx, NK_SYMBOL_TRIANGLE_LEFT, "prev", NK_TEXT_RIGHT)) {
        item_list_select_previous_node(item_list);
    }

    if (nk_button_symbol_label\
    (ctx, NK_SYMBOL_TRIANGLE_RIGHT, "next", NK_TEXT_LEFT)) {
        item_list_select_next_node(item_list);
    }

    nk_layout_row_dynamic(ctx, ENTERPRISE_WIDGET_HEIGHT, 1);
    // Create new item button.
    if (nk_button_label(ctx, "New Item")) {
        item_list_append(item_list);
    }

    // Create deletion button.
    // This will ask the user to confirm whether they want to delete the node.
    if (nk_button_label(ctx, "Delete Item")) {
        item_list->deletion_requested = true;
    }

    if (item_list->deletion_requested == true) {
        nk_label(ctx, "Confirm deletion?", NK_TEXT_CENTERED);
        nk_layout_row_dynamic(ctx, ENTERPRISE_WIDGET_HEIGHT, 2);

        if (nk_button_label(ctx, "Yes")) {
            item_list_delete_node(item_list, item->id);
            item_list->deletion_requested = false;
        }
        if (nk_button_label(ctx, "No")) {
            item_list->deletion_requested = false;
        }        
    }
    
    return program_status_item_editor;
}