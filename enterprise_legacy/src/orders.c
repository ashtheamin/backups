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

/* How orders work.
orders are stored in a struct that contains a pointer to the head of a
linked list containing all the orders. The struct that stores the linked
list is directly added to the enterprise struct and stores metadata about the
list. All orders are assigned a unique ID, no two orders can have the
same ID, the order_list structure keeps track of that.

Data structures:
order_node: An individual order.
order_list: A structure holding important metadata about the order
linked list.

order list metadata:
order_list->id_last_assigned: This is incremented by 1 every time a new
order is added in order to ensure no two orders ever have the same ID.

order_list->id_currently_selected: This is the ID that is selected in the
order editor dialogue.
*/

enum order_supplier_type {order_supplier_supplier, order_supplier_facility};
enum order_recipient_type {order_recipient_facility, order_recipient_customer};
struct order_node {
    char id[ENTERPRISE_STRING_LENGTH];
    char supplier_id[ENTERPRISE_STRING_LENGTH];
    char recipient_id[ENTERPRISE_STRING_LENGTH];

    enum order_supplier_type supplier_type;
    enum order_recipient_type recipient_type;

    time_t time_order_placed;
    bool delivered;

    struct order_node* prev;
    struct order_node* next;
};

// order node constructor and initialiser.
// Returns order node on success, or NULL on failure.
struct order_node *order_node_new() {
    struct order_node* order = malloc(sizeof(struct order_node));
    if (order == NULL) return NULL;
    strcpy(order->id, "");
    strcpy(order->supplier_id, "");
    strcpy(order->recipient_id, "");

    order->delivered = false;

    order->supplier_type = order_supplier_facility;
    order->recipient_type = order_recipient_customer;

    order->prev = NULL;
    order->next = NULL;

    return order;
}

// order list metadata structure.
struct order_list {
    struct order_node *head;
    char id_last_assigned[ENTERPRISE_STRING_LENGTH];
    char id_currently_selected[ENTERPRISE_STRING_LENGTH];
    bool deletion_requested;
};

// order list constructor.
// Returns order list on success, or NULL on failure.
struct order_list* order_list_new() {
    struct order_list* order_list = malloc(sizeof(struct order_list));
    if (order_list == NULL) return NULL;
    order_list->head = NULL;
    strcpy(order_list->id_last_assigned, "0");
    strcpy(order_list->id_currently_selected, "0");
    order_list->deletion_requested = false;
    return order_list;
}

// Free an entire order linked list
void order_node_linked_list_free(struct order_node* order) {
    if (order == NULL) return;
    struct order_node* next = NULL;
    while (order != NULL) {
        next = order->next;
        free(order);
        order = next;
    }
}

// Free all memory associated with a order list.
void order_list_free(struct order_list* order_list) {
    if (order_list == NULL) return;

    if (order_list->head != NULL) {
        order_node_linked_list_free(order_list->head);
    }

    free(order_list);
    return;
}

// Append a new order to a order list.
void order_list_append(struct order_list* order_list) {
    if (order_list == NULL) return;

    // Increment unique ID by 1.
    char buffer[ENTERPRISE_STRING_LENGTH];
    sprintf(buffer, "%lld", atoll(order_list->id_last_assigned) + 1);
    strcpy(order_list->id_last_assigned, buffer);

    // Add new node to start of order list if order linked list does
    // not exist.
    if (order_list->head == NULL) {
        order_list->head = order_node_new();
        strcpy(order_list->head->id, order_list->id_last_assigned);
        strcpy(order_list->id_currently_selected,
        order_list->id_last_assigned);
        return;
    }

    // Else add the new node to the end of the list.
    struct order_node* order = order_list->head;
    while (order->next != NULL) {
        order = order->next;
    }
    
    order->next = order_node_new();
    order->next->prev = order;

    strcpy(order->next->id, order_list->id_last_assigned);
    strcpy(order_list->id_currently_selected,
    order_list->id_last_assigned);

    return;
}

// Get the number of order nodes in the order list
int order_list_get_num_order_nodes(struct order_list* order_list) {
    if (order_list == NULL) return 0;
    if (order_list->head == NULL) return 0;

    struct order_node* order = order_list->head;
    int order_count = 0;
    while (order != NULL) {
        order_count++;
        order = order->next;
    }
    return order_count;
}

// Return a pointer to a order node according to ID.
// Returns NULL on failure.
struct order_node *order_list_get_node\
(struct order_list *order_list, char *id) {
    if (order_list == NULL || id == NULL) return NULL;
    if (order_list->head == NULL) return NULL;

    struct order_node* order = order_list->head;
    while (order != NULL) {
        if (strcmp(order->id, id) == 0) return order;
        order = order->next;
    }
    return NULL;
}

// Searches for a order by ID and deletes it
void order_list_delete_node(struct order_list *order_list, char *id) {
    if (order_list == NULL || id == NULL) return;
    if (order_list->head == NULL) return;

    // Delete the head if it is the ID that is requested to be deleted.
    if (strcmp(id, order_list->head->id) == 0) {
        if (order_list->head->next == NULL) {
            free(order_list->head);
            order_list->head = NULL;
            return;
        }

        struct order_node *next = order_list->head->next;
        free(order_list->head);
        order_list->head = next;
        return;
    }

    // Else search for the order to delete.
    struct order_node* order = order_list->head;
    while (order != NULL) {
        if (strcmp(order->id, id) == 0) break;
        order = order->next;
    }
    if (order == NULL) return;

    // Fix the pointers of the next and previous nodes to the order to be
    // deleted in order to ensure that traversal still works.

    struct order_node* prev = order->prev;
    struct order_node* next = order->next;

    if (prev != NULL) {
        if (next == NULL) {
            prev->next = NULL;
            strcpy(order_list->id_currently_selected, prev->id);
        }
        if (next != NULL) {
            prev->next = next;
            next->prev = prev;
            strcpy(order_list->id_currently_selected, next->id);
        }
    }

    if (prev == NULL) {
        if (next != NULL) {
            next->prev = NULL;
            strcpy(order_list->id_currently_selected, next->id);
        }
    }

    // Delete the order
    free(order);
    return;
}

// Select the previous node as the currently selected item.
void order_list_select_previous_node(struct order_list *order_list) {
    if (order_list == NULL) return;
    if (order_list->head == NULL) return;

    struct order_node* order = order_list->head;
    while (order != NULL) {
        if (strcmp(order->id, order_list->id_currently_selected) == 0) {
            if (order->prev != NULL) {
                strcpy(order_list->id_currently_selected, order->prev->id);
                return;
            }
            if (order->prev == NULL) {
                struct order_node* temp = order;
                while (temp->next != NULL) {
                    temp = temp->next;
                }
                strcpy(order_list->id_currently_selected, temp->id);
                return;
            }
        }
        order = order->next;
    }
}

// Select the next node as the currently selected item.
void order_list_select_next_node(struct order_list *order_list) {
    if (order_list == NULL) return;
    if (order_list->head == NULL) return;

    struct order_node* order = order_list->head;
    while (order != NULL) {
        if (strcmp(order->id, order_list->id_currently_selected) == 0) {
            if (order->next != NULL) {
                strcpy(order_list->id_currently_selected, order->next->id);
                return;
            }
            if (order->next == NULL) {
                strcpy(order_list->id_currently_selected,\
                order_list->head->id);
                return;
            }
        }
        order = order->next;
    }
}

// Change the currently selected ID to the passed in ID in the order list.
void order_list_set_selected_id\
(struct order_list *order_list, char* id) {
    if (order_list == NULL || id == NULL) return;
    if (order_list->head == NULL) return;

    struct order_node* order = order_list->head;
    while (order != NULL) {
        if (strcmp(order->id, id) == 0) {
            strcpy(order_list->id_currently_selected, id);
        }
        order = order->next;
    }
}

// Render the order table GUI.
// This function displays a list of orders as a table that can be selected.
// It is an overview.
enum program_status order_table(struct nk_context* ctx,\
struct order_list* order_list) {
    if (ctx == NULL || order_list == NULL) return program_status_running;

    // Button to return to enterprise menu.
    nk_layout_row_dynamic(ctx, ENTERPRISE_WIDGET_HEIGHT, 1);
    if (nk_button_label(ctx, "Return to Enterprise Menu")) {
        return program_status_enterprise_menu;
    }

    // Display the title.
    nk_label(ctx, "Order Menu", NK_TEXT_CENTERED);

    // Create button to add new orders to the table.
    if (nk_button_label(ctx, "New Order")) {
        order_list_append(order_list);
    }

    // If there are no orders, warn the user.
    if (order_list->head == NULL) {
        nk_label(ctx, "No Orders found.", NK_TEXT_CENTERED);
        return program_status_order_table;
    }

    /* If there are orders, go through the linked list of orders.
    Copy the data from each order and make it a button label.
    When the button is pressed, set the currently selected order to that
    order and switch to order editor.*/
    struct order_node* order = order_list->head;
    char* print_buffer = malloc(sizeof(char) * ENTERPRISE_STRING_LENGTH * 10);
    while (order != NULL) {
        sprintf(print_buffer, \
        "ID: %s Supplier ID: %s Recipient ID: %s",order->id,\
        order->supplier_id, order->recipient_id);

        if (nk_button_label(ctx, print_buffer)) {
            strcpy(order_list->id_currently_selected, order->id);
            free(print_buffer);
            return program_status_order_editor;
        }
        order = order->next;
    }
    free(print_buffer);
    return program_status_order_table;
}

// Render the order editor GUI.
// It gives the user an opportunity to edit a currently selected order.
enum program_status order_editor(struct nk_context* ctx,\
struct order_list* order_list) {
    if (ctx == NULL || order_list == NULL)
        return program_status_enterprise_menu;

    // Display title and return to order table button.
    nk_layout_row_dynamic(ctx, ENTERPRISE_WIDGET_HEIGHT, 1);
    if (nk_button_label(ctx, "Return to Order Table")) {
        return program_status_order_table;
    }
    nk_label(ctx, "Order Editor", NK_TEXT_CENTERED);
    
    // Select currently selected order.
    struct order_node* order = order_list_get_node\
    (order_list, order_list->id_currently_selected);

    // If the currently selected order does not exist:
    if (order == NULL) {
        // Select the first order instead.
        if (order_list->head != NULL) {
            strcpy(order_list->id_currently_selected, order_list->head->id);
            return program_status_order_editor;
        }

        // Else tell the user to create a new order 
        // if the first order doesn't exist.
        else {
            nk_label(ctx, "No Orders found.", NK_TEXT_CENTERED);
            if (nk_button_label(ctx, "New Order")) {
                order_list_append(order_list);
            }
            return program_status_order_editor;
        }
    }

    // Display edit fields to edit order entries.
    nk_layout_row_template_begin(ctx, ENTERPRISE_WIDGET_HEIGHT);
    nk_layout_row_template_push_static(ctx, 150);
    nk_layout_row_template_push_dynamic(ctx);
    nk_layout_row_template_end(ctx);

    nk_label(ctx, "ID: ", NK_TEXT_LEFT);
    nk_edit_string_zero_terminated(ctx, NK_EDIT_READ_ONLY, \
    order->id, ENTERPRISE_STRING_LENGTH, nk_filter_default);

    nk_label(ctx, "Supplier ID: ", NK_TEXT_LEFT);
    nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, \
    order->supplier_id, ENTERPRISE_STRING_LENGTH, nk_filter_default);

    nk_label(ctx, "Recipient ID: ", NK_TEXT_LEFT);
    nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, \
    order->recipient_id, ENTERPRISE_STRING_LENGTH, nk_filter_default);

    // Move between next and previous orders.
    nk_layout_row_dynamic(ctx, ENTERPRISE_WIDGET_HEIGHT, 2);
    if (nk_button_symbol_label\
    (ctx, NK_SYMBOL_TRIANGLE_LEFT, "prev", NK_TEXT_RIGHT)) {
        order_list_select_previous_node(order_list);
    }

    if (nk_button_symbol_label\
    (ctx, NK_SYMBOL_TRIANGLE_RIGHT, "next", NK_TEXT_LEFT)) {
        order_list_select_next_node(order_list);
    }

    nk_layout_row_dynamic(ctx, ENTERPRISE_WIDGET_HEIGHT, 1);
    // Create new order button.
    if (nk_button_label(ctx, "New Order")) {
        order_list_append(order_list);
    }

    // Create deletion button.
    // This will ask the user to confirm whether they want to delete the node.
    if (nk_button_label(ctx, "Delete Order")) {
        order_list->deletion_requested = true;
    }

    if (order_list->deletion_requested == true) {
        nk_label(ctx, "Confirm deletion?", NK_TEXT_CENTERED);
        nk_layout_row_dynamic(ctx, ENTERPRISE_WIDGET_HEIGHT, 2);

        if (nk_button_label(ctx, "Yes")) {
            order_list_delete_node(order_list, order->id);
            order_list->deletion_requested = false;
        }
        if (nk_button_label(ctx, "No")) {
            order_list->deletion_requested = false;
        }        
    }
    
    return program_status_order_editor;
}