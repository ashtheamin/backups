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

/* How customers work.
customers are stored in a struct that contains a pointer to the head of a
linked list containing all the customers. The struct that stores the linked
list is directly added to the enterprise struct and stores metadata about the
list. All customers are assigned a unique ID, no two customers can have the
same ID, the customer_list structure keeps track of that.

Data structures:
customer_node: An individual customer.
customer_list: A structure holding important metadata about the customer
linked list.

customer list metadata:
customer_list->id_last_assigned: This is incremented by 1 every time a new
customer is added in order to ensure no two customers ever have the same ID.

customer_list->id_currently_selected: This is the ID that is selected in the
customer editor dialogue.
*/

struct customer_node {
    char id[ENTERPRISE_STRING_LENGTH];
    char name[ENTERPRISE_STRING_LENGTH];
    char email[ENTERPRISE_STRING_LENGTH];
    char phone[ENTERPRISE_STRING_LENGTH];
    char address[ENTERPRISE_STRING_LENGTH];

    struct customer_node* prev;
    struct customer_node* next;
};

// customer node constructor and initialiser.
// Returns customer node on success, or NULL on failure.
struct customer_node *customer_node_new() {
    struct customer_node* customer = malloc(sizeof(struct customer_node));
    if (customer == NULL) return NULL;
    strcpy(customer->id, "");
    strcpy(customer->name, "");
    strcpy(customer->email, "");
    strcpy(customer->phone, "");
    strcpy(customer->address, "");

    customer->prev = NULL;
    customer->next = NULL;

    return customer;
}

// customer list metadata structure.
struct customer_list {
    struct customer_node *head;
    char id_last_assigned[ENTERPRISE_STRING_LENGTH];
    char id_currently_selected[ENTERPRISE_STRING_LENGTH];
    bool deletion_requested;
};

// customer list constructor.
// Returns customer list on success, or NULL on failure.
struct customer_list* customer_list_new() {
    struct customer_list* customer_list = malloc(sizeof(struct customer_list));
    if (customer_list == NULL) return NULL;
    customer_list->head = NULL;
    strcpy(customer_list->id_last_assigned, "0");
    strcpy(customer_list->id_currently_selected, "0");
    customer_list->deletion_requested = false;
    return customer_list;
}

// Free an entire customer linked list
void customer_node_linked_list_free(struct customer_node* customer) {
    if (customer == NULL) return;
    struct customer_node* next = NULL;
    while (customer != NULL) {
        next = customer->next;
        free(customer);
        customer = next;
    }
}

// Free all memory associated with a customer list.
void customer_list_free(struct customer_list* customer_list) {
    if (customer_list == NULL) return;

    if (customer_list->head != NULL) {
        customer_node_linked_list_free(customer_list->head);
    }

    free(customer_list);
    return;
}

// Append a new customer to a customer list.
void customer_list_append(struct customer_list* customer_list) {
    if (customer_list == NULL) return;

    // Increment unique ID by 1.
    char buffer[ENTERPRISE_STRING_LENGTH];
    sprintf(buffer, "%lld", atoll(customer_list->id_last_assigned) + 1);
    strcpy(customer_list->id_last_assigned, buffer);

    // Add new node to start of customer list if customer linked list does
    // not exist.
    if (customer_list->head == NULL) {
        customer_list->head = customer_node_new();
        strcpy(customer_list->head->id, customer_list->id_last_assigned);
        strcpy(customer_list->id_currently_selected,
        customer_list->id_last_assigned);
        return;
    }

    // Else add the new node to the end of the list.
    struct customer_node* customer = customer_list->head;
    while (customer->next != NULL) {
        customer = customer->next;
    }
    
    customer->next = customer_node_new();
    customer->next->prev = customer;

    strcpy(customer->next->id, customer_list->id_last_assigned);
    strcpy(customer_list->id_currently_selected,
    customer_list->id_last_assigned);

    return;
}

// Get the number of customer nodes in the customer list
int customer_list_get_num_customer_nodes(struct customer_list* customer_list) {
    if (customer_list == NULL) return 0;
    if (customer_list->head == NULL) return 0;

    struct customer_node* customer = customer_list->head;
    int customer_count = 0;
    while (customer != NULL) {
        customer_count++;
        customer = customer->next;
    }
    return customer_count;
}

// Return a pointer to a customer node according to ID.
// Returns NULL on failure.
struct customer_node *customer_list_get_node\
(struct customer_list *customer_list, char *id) {
    if (customer_list == NULL || id == NULL) return NULL;
    if (customer_list->head == NULL) return NULL;

    struct customer_node* customer = customer_list->head;
    while (customer != NULL) {
        if (strcmp(customer->id, id) == 0) return customer;
        customer = customer->next;
    }
    return NULL;
}

// Searches for a customer by ID and deletes it
void customer_list_delete_node(struct customer_list *customer_list, char *id) {
    if (customer_list == NULL || id == NULL) return;
    if (customer_list->head == NULL) return;

    // Delete the head if it is the ID that is requested to be deleted.
    if (strcmp(id, customer_list->head->id) == 0) {
        if (customer_list->head->next == NULL) {
            free(customer_list->head);
            customer_list->head = NULL;
            return;
        }

        struct customer_node *next = customer_list->head->next;
        free(customer_list->head);
        customer_list->head = next;
        return;
    }

    // Else search for the customer to delete.
    struct customer_node* customer = customer_list->head;
    while (customer != NULL) {
        if (strcmp(customer->id, id) == 0) break;
        customer = customer->next;
    }
    if (customer == NULL) return;

    // Fix the pointers of the next and previous nodes to the customer to be
    // deleted in order to ensure that traversal still works.

    struct customer_node* prev = customer->prev;
    struct customer_node* next = customer->next;

    if (prev != NULL) {
        if (next == NULL) {
            prev->next = NULL;
            strcpy(customer_list->id_currently_selected, prev->id);
        }
        if (next != NULL) {
            prev->next = next;
            next->prev = prev;
            strcpy(customer_list->id_currently_selected, next->id);
        }
    }

    if (prev == NULL) {
        if (next != NULL) {
            next->prev = NULL;
            strcpy(customer_list->id_currently_selected, next->id);
        }
    }

    // Delete the customer
    free(customer);
    return;
}

// Select the previous node as the currently selected item.
void customer_list_select_previous_node(struct customer_list *customer_list) {
    if (customer_list == NULL) return;
    if (customer_list->head == NULL) return;

    struct customer_node* customer = customer_list->head;
    while (customer != NULL) {
        if (strcmp(customer->id, customer_list->id_currently_selected) == 0) {
            if (customer->prev != NULL) {
                strcpy(customer_list->id_currently_selected, customer->prev->id);
                return;
            }
            if (customer->prev == NULL) {
                struct customer_node* temp = customer;
                while (temp->next != NULL) {
                    temp = temp->next;
                }
                strcpy(customer_list->id_currently_selected, temp->id);
                return;
            }
        }
        customer = customer->next;
    }
}

// Select the next node as the currently selected item.
void customer_list_select_next_node(struct customer_list *customer_list) {
    if (customer_list == NULL) return;
    if (customer_list->head == NULL) return;

    struct customer_node* customer = customer_list->head;
    while (customer != NULL) {
        if (strcmp(customer->id, customer_list->id_currently_selected) == 0) {
            if (customer->next != NULL) {
                strcpy(customer_list->id_currently_selected, customer->next->id);
                return;
            }
            if (customer->next == NULL) {
                strcpy(customer_list->id_currently_selected,\
                customer_list->head->id);
                return;
            }
        }
        customer = customer->next;
    }
}

// Change the currently selected ID to the passed in ID in the customer list.
void customer_list_set_selected_id\
(struct customer_list *customer_list, char* id) {
    if (customer_list == NULL || id == NULL) return;
    if (customer_list->head == NULL) return;

    struct customer_node* customer = customer_list->head;
    while (customer != NULL) {
        if (strcmp(customer->id, id) == 0) {
            strcpy(customer_list->id_currently_selected, id);
        }
        customer = customer->next;
    }
}

// Render the customer table GUI.
// This function displays a list of customers as a table that can be selected.
// It is an overview.
enum program_status customer_table(struct nk_context* ctx,\
struct customer_list* customer_list) {
    if (ctx == NULL || customer_list == NULL) return program_status_running;

    // Button to return to enterprise menu.
    nk_layout_row_dynamic(ctx, ENTERPRISE_WIDGET_HEIGHT, 1);
    if (nk_button_label(ctx, "Return to Enterprise Menu")) {
        return program_status_enterprise_menu;
    }

    // Display the title.
    nk_label(ctx, "Customer Menu", NK_TEXT_CENTERED);

    // Create button to add new customers to the table.
    if (nk_button_label(ctx, "New Customer")) {
        customer_list_append(customer_list);
    }

    // If there are no customers, warn the user.
    if (customer_list->head == NULL) {
        nk_label(ctx, "No customers found.", NK_TEXT_CENTERED);
        return program_status_customer_table;
    }

    /* If there are customers, go through the linked list of customers.
    Copy the data from each customer and make it a button label.
    When the button is pressed, set the currently selected customer to that
    customer and switch to customer editor.*/
    struct customer_node* customer = customer_list->head;
    char* print_buffer = malloc(sizeof(char) * ENTERPRISE_STRING_LENGTH * 10);
    while (customer != NULL) {
        sprintf(print_buffer, \
        "ID: %s Name: %s Email: %s Phone: %s Address: %s",customer->id,\
        customer->name, customer->email, customer->phone, customer->address);

        if (nk_button_label(ctx, print_buffer)) {
            strcpy(customer_list->id_currently_selected, customer->id);
            free(print_buffer);
            return program_status_customer_editor;
        }
        customer = customer->next;
    }
    free(print_buffer);
    return program_status_customer_table;
}

// Render the customer editor GUI.
// It gives the user an opportunity to edit a currently selected customer.
enum program_status customer_editor(struct nk_context* ctx,\
struct customer_list* customer_list) {
    if (ctx == NULL || customer_list == NULL)
        return program_status_enterprise_menu;

    // Display title and return to customer table button.
    nk_layout_row_dynamic(ctx, ENTERPRISE_WIDGET_HEIGHT, 1);
    if (nk_button_label(ctx, "Return to Customer Table")) {
        return program_status_customer_table;
    }
    nk_label(ctx, "Customer Editor", NK_TEXT_CENTERED);
    
    // Select currently selected customer.
    struct customer_node* customer = customer_list_get_node\
    (customer_list, customer_list->id_currently_selected);

    // If the currently selected customer does not exist:
    if (customer == NULL) {
        // Select the first customer instead.
        if (customer_list->head != NULL) {
            strcpy(customer_list->id_currently_selected, customer_list->head->id);
            return program_status_customer_editor;
        }

        // Else tell the user to create a new customer 
        // if the first customer doesn't exist.
        else {
            nk_label(ctx, "No Customers found.", NK_TEXT_CENTERED);
            if (nk_button_label(ctx, "New Customer")) {
                customer_list_append(customer_list);
            }
            return program_status_customer_editor;
        }
    }

    // Display edit fields to edit customer entries.
    nk_layout_row_template_begin(ctx, ENTERPRISE_WIDGET_HEIGHT);
    nk_layout_row_template_push_static(ctx, 150);
    nk_layout_row_template_push_dynamic(ctx);
    nk_layout_row_template_end(ctx);

    nk_label(ctx, "ID: ", NK_TEXT_LEFT);
    nk_edit_string_zero_terminated(ctx, NK_EDIT_READ_ONLY, \
    customer->id, ENTERPRISE_STRING_LENGTH, nk_filter_default);

    nk_label(ctx, "Name: ", NK_TEXT_LEFT);
    nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, \
    customer->name, ENTERPRISE_STRING_LENGTH, nk_filter_default);

    nk_label(ctx, "Phone: ", NK_TEXT_LEFT);
    nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, \
    customer->phone, ENTERPRISE_STRING_LENGTH, nk_filter_default);

    nk_label(ctx, "Email: ", NK_TEXT_LEFT);
    nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, \
    customer->email, ENTERPRISE_STRING_LENGTH, nk_filter_default);

    nk_label(ctx, "Address: ", NK_TEXT_LEFT);
    nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, \
    customer->address, ENTERPRISE_STRING_LENGTH, nk_filter_default);

    // Move between next and previous customers.
    nk_layout_row_dynamic(ctx, ENTERPRISE_WIDGET_HEIGHT, 2);
    if (nk_button_symbol_label\
    (ctx, NK_SYMBOL_TRIANGLE_LEFT, "prev", NK_TEXT_RIGHT)) {
        customer_list_select_previous_node(customer_list);
    }

    if (nk_button_symbol_label\
    (ctx, NK_SYMBOL_TRIANGLE_RIGHT, "next", NK_TEXT_LEFT)) {
        customer_list_select_next_node(customer_list);
    }

    nk_layout_row_dynamic(ctx, ENTERPRISE_WIDGET_HEIGHT, 1);
    // Create new customer button.
    if (nk_button_label(ctx, "New Customer")) {
        customer_list_append(customer_list);
    }

    // Create deletion button.
    // This will ask the user to confirm whether they want to delete the node.
    if (nk_button_label(ctx, "Delete Customer")) {
        customer_list->deletion_requested = true;
    }

    if (customer_list->deletion_requested == true) {
        nk_label(ctx, "Confirm deletion?", NK_TEXT_CENTERED);
        nk_layout_row_dynamic(ctx, ENTERPRISE_WIDGET_HEIGHT, 2);

        if (nk_button_label(ctx, "Yes")) {
            customer_list_delete_node(customer_list, customer->id);
            customer_list->deletion_requested = false;
        }
        if (nk_button_label(ctx, "No")) {
            customer_list->deletion_requested = false;
        }        
    }
    
    return program_status_customer_editor;
}