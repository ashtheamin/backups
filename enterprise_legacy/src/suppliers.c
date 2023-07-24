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

/* How suppliers work.
suppliers are stored in a struct that contains a pointer to the head of a
linked list containing all the suppliers. The struct that stores the linked
list is directly added to the enterprise struct and stores metadata about the
list. All suppliers are assigned a unique ID, no two suppliers can have the
same ID, the supplier_list structure keeps track of that.

Data structures:
supplier_node: An individual supplier.
supplier_list: A structure holding important metadata about the supplier
linked list.

supplier list metadata:
supplier_list->id_last_assigned: This is incremented by 1 every time a new
supplier is added in order to ensure no two suppliers ever have the same ID.

supplier_list->id_currently_selected: This is the ID that is selected in the
supplier editor dialogue.
*/

struct supplier_node {
    char id[ENTERPRISE_STRING_LENGTH];
    char name[ENTERPRISE_STRING_LENGTH];
    char email[ENTERPRISE_STRING_LENGTH];
    char phone[ENTERPRISE_STRING_LENGTH];
    char address[ENTERPRISE_STRING_LENGTH];

    struct supplier_node* prev;
    struct supplier_node* next;
};

// supplier node constructor and initialiser.
// Returns supplier node on success, or NULL on failure.
struct supplier_node *supplier_node_new() {
    struct supplier_node* supplier = malloc(sizeof(struct supplier_node));
    if (supplier == NULL) return NULL;
    strcpy(supplier->id, "");
    strcpy(supplier->name, "");
    strcpy(supplier->email, "");
    strcpy(supplier->phone, "");
    strcpy(supplier->address, "");

    supplier->prev = NULL;
    supplier->next = NULL;

    return supplier;
}

// supplier list metadata structure.
struct supplier_list {
    struct supplier_node *head;
    char id_last_assigned[ENTERPRISE_STRING_LENGTH];
    char id_currently_selected[ENTERPRISE_STRING_LENGTH];
    bool deletion_requested;
};

// supplier list constructor.
// Returns supplier list on success, or NULL on failure.
struct supplier_list* supplier_list_new() {
    struct supplier_list* supplier_list = malloc(sizeof(struct supplier_list));
    if (supplier_list == NULL) return NULL;
    supplier_list->head = NULL;
    strcpy(supplier_list->id_last_assigned, "0");
    strcpy(supplier_list->id_currently_selected, "0");
    supplier_list->deletion_requested = false;
    return supplier_list;
}

// Free an entire supplier linked list
void supplier_node_linked_list_free(struct supplier_node* supplier) {
    if (supplier == NULL) return;
    struct supplier_node* next = NULL;
    while (supplier != NULL) {
        next = supplier->next;
        free(supplier);
        supplier = next;
    }
}

// Free all memory associated with a supplier list.
void supplier_list_free(struct supplier_list* supplier_list) {
    if (supplier_list == NULL) return;

    if (supplier_list->head != NULL) {
        supplier_node_linked_list_free(supplier_list->head);
    }

    free(supplier_list);
    return;
}

// Append a new supplier to a supplier list.
void supplier_list_append(struct supplier_list* supplier_list) {
    if (supplier_list == NULL) return;

    // Increment unique ID by 1.
    char buffer[ENTERPRISE_STRING_LENGTH];
    sprintf(buffer, "%lld", atoll(supplier_list->id_last_assigned) + 1);
    strcpy(supplier_list->id_last_assigned, buffer);

    // Add new node to start of supplier list if supplier linked list does
    // not exist.
    if (supplier_list->head == NULL) {
        supplier_list->head = supplier_node_new();
        strcpy(supplier_list->head->id, supplier_list->id_last_assigned);
        strcpy(supplier_list->id_currently_selected,
        supplier_list->id_last_assigned);
        return;
    }

    // Else add the new node to the end of the list.
    struct supplier_node* supplier = supplier_list->head;
    while (supplier->next != NULL) {
        supplier = supplier->next;
    }
    
    supplier->next = supplier_node_new();
    supplier->next->prev = supplier;

    strcpy(supplier->next->id, supplier_list->id_last_assigned);
    strcpy(supplier_list->id_currently_selected,
    supplier_list->id_last_assigned);

    return;
}

// Get the number of supplier nodes in the supplier list
int supplier_list_get_num_supplier_nodes(struct supplier_list* supplier_list) {
    if (supplier_list == NULL) return 0;
    if (supplier_list->head == NULL) return 0;

    struct supplier_node* supplier = supplier_list->head;
    int supplier_count = 0;
    while (supplier != NULL) {
        supplier_count++;
        supplier = supplier->next;
    }
    return supplier_count;
}

// Return a pointer to a supplier node according to ID.
// Returns NULL on failure.
struct supplier_node *supplier_list_get_node\
(struct supplier_list *supplier_list, char *id) {
    if (supplier_list == NULL || id == NULL) return NULL;
    if (supplier_list->head == NULL) return NULL;

    struct supplier_node* supplier = supplier_list->head;
    while (supplier != NULL) {
        if (strcmp(supplier->id, id) == 0) return supplier;
        supplier = supplier->next;
    }
    return NULL;
}

// Searches for a supplier by ID and deletes it
void supplier_list_delete_node(struct supplier_list *supplier_list, char *id) {
    if (supplier_list == NULL || id == NULL) return;
    if (supplier_list->head == NULL) return;

    // Delete the head if it is the ID that is requested to be deleted.
    if (strcmp(id, supplier_list->head->id) == 0) {
        if (supplier_list->head->next == NULL) {
            free(supplier_list->head);
            supplier_list->head = NULL;
            return;
        }

        struct supplier_node *next = supplier_list->head->next;
        free(supplier_list->head);
        supplier_list->head = next;
        return;
    }

    // Else search for the supplier to delete.
    struct supplier_node* supplier = supplier_list->head;
    while (supplier != NULL) {
        if (strcmp(supplier->id, id) == 0) break;
        supplier = supplier->next;
    }
    if (supplier == NULL) return;

    // Fix the pointers of the next and previous nodes to the supplier to be
    // deleted in order to ensure that traversal still works.

    struct supplier_node* prev = supplier->prev;
    struct supplier_node* next = supplier->next;

    if (prev != NULL) {
        if (next == NULL) {
            prev->next = NULL;
            strcpy(supplier_list->id_currently_selected, prev->id);
        }
        if (next != NULL) {
            prev->next = next;
            next->prev = prev;
            strcpy(supplier_list->id_currently_selected, next->id);
        }
    }

    if (prev == NULL) {
        if (next != NULL) {
            next->prev = NULL;
            strcpy(supplier_list->id_currently_selected, next->id);
        }
    }

    // Delete the supplier
    free(supplier);
    return;
}

// Select the previous node as the currently selected item.
void supplier_list_select_previous_node(struct supplier_list *supplier_list) {
    if (supplier_list == NULL) return;
    if (supplier_list->head == NULL) return;

    struct supplier_node* supplier = supplier_list->head;
    while (supplier != NULL) {
        if (strcmp(supplier->id, supplier_list->id_currently_selected) == 0) {
            if (supplier->prev != NULL) {
                strcpy(supplier_list->id_currently_selected, supplier->prev->id);
                return;
            }
            if (supplier->prev == NULL) {
                struct supplier_node* temp = supplier;
                while (temp->next != NULL) {
                    temp = temp->next;
                }
                strcpy(supplier_list->id_currently_selected, temp->id);
                return;
            }
        }
        supplier = supplier->next;
    }
}

// Select the next node as the currently selected item.
void supplier_list_select_next_node(struct supplier_list *supplier_list) {
    if (supplier_list == NULL) return;
    if (supplier_list->head == NULL) return;

    struct supplier_node* supplier = supplier_list->head;
    while (supplier != NULL) {
        if (strcmp(supplier->id, supplier_list->id_currently_selected) == 0) {
            if (supplier->next != NULL) {
                strcpy(supplier_list->id_currently_selected, supplier->next->id);
                return;
            }
            if (supplier->next == NULL) {
                strcpy(supplier_list->id_currently_selected,\
                supplier_list->head->id);
                return;
            }
        }
        supplier = supplier->next;
    }
}

// Change the currently selected ID to the passed in ID in the supplier list.
void supplier_list_set_selected_id\
(struct supplier_list *supplier_list, char* id) {
    if (supplier_list == NULL || id == NULL) return;
    if (supplier_list->head == NULL) return;

    struct supplier_node* supplier = supplier_list->head;
    while (supplier != NULL) {
        if (strcmp(supplier->id, id) == 0) {
            strcpy(supplier_list->id_currently_selected, id);
        }
        supplier = supplier->next;
    }
}

// Render the supplier table GUI.
// This function displays a list of suppliers as a table that can be selected.
// It is an overview.
enum program_status supplier_table(struct nk_context* ctx,\
struct supplier_list* supplier_list) {
    if (ctx == NULL || supplier_list == NULL) return program_status_running;

    // Button to return to enterprise menu.
    nk_layout_row_dynamic(ctx, ENTERPRISE_WIDGET_HEIGHT, 1);
    if (nk_button_label(ctx, "Return to Enterprise Menu")) {
        return program_status_enterprise_menu;
    }

    // Display the title.
    nk_label(ctx, "Supplier Menu", NK_TEXT_CENTERED);

    // Create button to add new suppliers to the table.
    if (nk_button_label(ctx, "New Supplier")) {
        supplier_list_append(supplier_list);
    }

    // If there are no suppliers, warn the user.
    if (supplier_list->head == NULL) {
        nk_label(ctx, "No Suppliers found.", NK_TEXT_CENTERED);
        return program_status_supplier_table;
    }

    /* If there are suppliers, go through the linked list of suppliers.
    Copy the data from each supplier and make it a button label.
    When the button is pressed, set the currently selected supplier to that
    supplier and switch to supplier editor.*/
    struct supplier_node* supplier = supplier_list->head;
    char* print_buffer = malloc(sizeof(char) * ENTERPRISE_STRING_LENGTH * 10);
    while (supplier != NULL) {
        sprintf(print_buffer, \
        "ID: %s Name: %s Email: %s Phone: %s Address: %s",supplier->id,\
        supplier->name, supplier->email, supplier->phone, supplier->address);

        if (nk_button_label(ctx, print_buffer)) {
            strcpy(supplier_list->id_currently_selected, supplier->id);
            free(print_buffer);
            return program_status_supplier_editor;
        }
        supplier = supplier->next;
    }
    free(print_buffer);
    return program_status_supplier_table;
}

// Render the supplier editor GUI.
// It gives the user an opportunity to edit a currently selected supplier.
enum program_status supplier_editor(struct nk_context* ctx,\
struct supplier_list* supplier_list) {
    if (ctx == NULL || supplier_list == NULL)
        return program_status_enterprise_menu;

    // Display title and return to supplier table button.
    nk_layout_row_dynamic(ctx, ENTERPRISE_WIDGET_HEIGHT, 1);
    if (nk_button_label(ctx, "Return to Supplier Table")) {
        return program_status_supplier_table;
    }
    nk_label(ctx, "Supplier Editor", NK_TEXT_CENTERED);
    
    // Select currently selected supplier.
    struct supplier_node* supplier = supplier_list_get_node\
    (supplier_list, supplier_list->id_currently_selected);

    // If the currently selected supplier does not exist:
    if (supplier == NULL) {
        // Select the first supplier instead.
        if (supplier_list->head != NULL) {
            strcpy(supplier_list->id_currently_selected, supplier_list->head->id);
            return program_status_supplier_editor;
        }

        // Else tell the user to create a new supplier 
        // if the first supplier doesn't exist.
        else {
            nk_label(ctx, "No Suppliers found.", NK_TEXT_CENTERED);
            if (nk_button_label(ctx, "New Supplier")) {
                supplier_list_append(supplier_list);
            }
            return program_status_supplier_editor;
        }
    }

    // Display edit fields to edit supplier entries.
    nk_layout_row_template_begin(ctx, ENTERPRISE_WIDGET_HEIGHT);
    nk_layout_row_template_push_static(ctx, 150);
    nk_layout_row_template_push_dynamic(ctx);
    nk_layout_row_template_end(ctx);

    nk_label(ctx, "ID: ", NK_TEXT_LEFT);
    nk_edit_string_zero_terminated(ctx, NK_EDIT_READ_ONLY, \
    supplier->id, ENTERPRISE_STRING_LENGTH, nk_filter_default);

    nk_label(ctx, "Name: ", NK_TEXT_LEFT);
    nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, \
    supplier->name, ENTERPRISE_STRING_LENGTH, nk_filter_default);

    nk_label(ctx, "Phone: ", NK_TEXT_LEFT);
    nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, \
    supplier->phone, ENTERPRISE_STRING_LENGTH, nk_filter_default);

    nk_label(ctx, "Email: ", NK_TEXT_LEFT);
    nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, \
    supplier->email, ENTERPRISE_STRING_LENGTH, nk_filter_default);

    nk_label(ctx, "Address: ", NK_TEXT_LEFT);
    nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, \
    supplier->address, ENTERPRISE_STRING_LENGTH, nk_filter_default);

    // Move between next and previous suppliers.
    nk_layout_row_dynamic(ctx, ENTERPRISE_WIDGET_HEIGHT, 2);
    if (nk_button_symbol_label\
    (ctx, NK_SYMBOL_TRIANGLE_LEFT, "prev", NK_TEXT_RIGHT)) {
        supplier_list_select_previous_node(supplier_list);
    }

    if (nk_button_symbol_label\
    (ctx, NK_SYMBOL_TRIANGLE_RIGHT, "next", NK_TEXT_LEFT)) {
        supplier_list_select_next_node(supplier_list);
    }

    nk_layout_row_dynamic(ctx, ENTERPRISE_WIDGET_HEIGHT, 1);
    // Create new supplier button.
    if (nk_button_label(ctx, "New Supplier")) {
        supplier_list_append(supplier_list);
    }

    // Create deletion button.
    // This will ask the user to confirm whether they want to delete the node.
    if (nk_button_label(ctx, "Delete Supplier")) {
        supplier_list->deletion_requested = true;
    }

    if (supplier_list->deletion_requested == true) {
        nk_label(ctx, "Confirm deletion?", NK_TEXT_CENTERED);
        nk_layout_row_dynamic(ctx, ENTERPRISE_WIDGET_HEIGHT, 2);

        if (nk_button_label(ctx, "Yes")) {
            supplier_list_delete_node(supplier_list, supplier->id);
            supplier_list->deletion_requested = false;
        }
        if (nk_button_label(ctx, "No")) {
            supplier_list->deletion_requested = false;
        }        
    }
    
    return program_status_supplier_editor;
}