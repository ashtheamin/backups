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

/* How expenses work.
expenses are stored in a struct that contains a pointer to the head of a
linked list containing all the expenses. The struct that stores the linked
list is directly added to the enterprise struct and stores metadata about the
list. All expenses are assigned a unique ID, no two expenses can have the
same ID, the expense_list structure keeps track of that.

Data structures:
expense_node: An individual expense.
expense_list: A structure holding important metadata about the expense
linked list.

expense list metadata:
expense_list->id_last_assigned: This is incremented by 1 every time a new
expense is added in order to ensure no two expenses ever have the same ID.

expense_list->id_currently_selected: This is the ID that is selected in the
expense editor dialogue.
*/

enum expense_type {expense_type_rent, expense_type_wage, expense_type_insurance,
expense_type_energy, expense_type_misc};

struct expense_node {
    char id[ENTERPRISE_STRING_LENGTH];
    char facility_id[ENTERPRISE_STRING_LENGTH];
    char supplier_id[ENTERPRISE_STRING_LENGTH];
    enum expense_type type;

    struct expense_node* prev;
    struct expense_node* next;
};

// expense node constructor and initialiser.
// Returns expense node on success, or NULL on failure.
struct expense_node *expense_node_new() {
    struct expense_node* expense = malloc(sizeof(struct expense_node));
    if (expense == NULL) return NULL;
    strcpy(expense->id, "");
    strcpy(expense->facility_id, "");
    strcpy(expense->supplier_id, "");
    expense->type = expense_type_misc;

    expense->prev = NULL;
    expense->next = NULL;

    return expense;
}

// expense list metadata structure.
struct expense_list {
    struct expense_node *head;
    char id_last_assigned[ENTERPRISE_STRING_LENGTH];
    char id_currently_selected[ENTERPRISE_STRING_LENGTH];
    bool deletion_requested;
};

// expense list constructor.
// Returns expense list on success, or NULL on failure.
struct expense_list* expense_list_new() {
    struct expense_list* expense_list = malloc(sizeof(struct expense_list));
    if (expense_list == NULL) return NULL;
    expense_list->head = NULL;
    strcpy(expense_list->id_last_assigned, "0");
    strcpy(expense_list->id_currently_selected, "0");
    expense_list->deletion_requested = false;
    return expense_list;
}

// Free an entire expense linked list
void expense_node_linked_list_free(struct expense_node* expense) {
    if (expense == NULL) return;
    struct expense_node* next = NULL;
    while (expense != NULL) {
        next = expense->next;
        free(expense);
        expense = next;
    }
}

// Free all memory associated with a expense list.
void expense_list_free(struct expense_list* expense_list) {
    if (expense_list == NULL) return;

    if (expense_list->head != NULL) {
        expense_node_linked_list_free(expense_list->head);
    }

    free(expense_list);
    return;
}

// Append a new expense to a expense list.
void expense_list_append(struct expense_list* expense_list) {
    if (expense_list == NULL) return;

    // Increment unique ID by 1.
    char buffer[ENTERPRISE_STRING_LENGTH];
    sprintf(buffer, "%lld", atoll(expense_list->id_last_assigned) + 1);
    strcpy(expense_list->id_last_assigned, buffer);

    // Add new node to start of expense list if expense linked list does
    // not exist.
    if (expense_list->head == NULL) {
        expense_list->head = expense_node_new();
        strcpy(expense_list->head->id, expense_list->id_last_assigned);
        strcpy(expense_list->id_currently_selected,
        expense_list->id_last_assigned);
        return;
    }

    // Else add the new node to the end of the list.
    struct expense_node* expense = expense_list->head;
    while (expense->next != NULL) {
        expense = expense->next;
    }
    
    expense->next = expense_node_new();
    expense->next->prev = expense;

    strcpy(expense->next->id, expense_list->id_last_assigned);
    strcpy(expense_list->id_currently_selected,
    expense_list->id_last_assigned);

    return;
}

// Get the number of expense nodes in the expense list
int expense_list_get_num_expense_nodes(struct expense_list* expense_list) {
    if (expense_list == NULL) return 0;
    if (expense_list->head == NULL) return 0;

    struct expense_node* expense = expense_list->head;
    int expense_count = 0;
    while (expense != NULL) {
        expense_count++;
        expense = expense->next;
    }
    return expense_count;
}

// Return a pointer to a expense node according to ID.
// Returns NULL on failure.
struct expense_node *expense_list_get_node\
(struct expense_list *expense_list, char *id) {
    if (expense_list == NULL || id == NULL) return NULL;
    if (expense_list->head == NULL) return NULL;

    struct expense_node* expense = expense_list->head;
    while (expense != NULL) {
        if (strcmp(expense->id, id) == 0) return expense;
        expense = expense->next;
    }
    return NULL;
}

// Searches for a expense by ID and deletes it
void expense_list_delete_node(struct expense_list *expense_list, char *id) {
    if (expense_list == NULL || id == NULL) return;
    if (expense_list->head == NULL) return;

    // Delete the head if it is the ID that is requested to be deleted.
    if (strcmp(id, expense_list->head->id) == 0) {
        if (expense_list->head->next == NULL) {
            free(expense_list->head);
            expense_list->head = NULL;
            return;
        }

        struct expense_node *next = expense_list->head->next;
        free(expense_list->head);
        expense_list->head = next;
        return;
    }

    // Else search for the expense to delete.
    struct expense_node* expense = expense_list->head;
    while (expense != NULL) {
        if (strcmp(expense->id, id) == 0) break;
        expense = expense->next;
    }
    if (expense == NULL) return;

    // Fix the pointers of the next and previous nodes to the expense to be
    // deleted in order to ensure that traversal still works.

    struct expense_node* prev = expense->prev;
    struct expense_node* next = expense->next;

    if (prev != NULL) {
        if (next == NULL) {
            prev->next = NULL;
            strcpy(expense_list->id_currently_selected, prev->id);
        }
        if (next != NULL) {
            prev->next = next;
            next->prev = prev;
            strcpy(expense_list->id_currently_selected, next->id);
        }
    }

    if (prev == NULL) {
        if (next != NULL) {
            next->prev = NULL;
            strcpy(expense_list->id_currently_selected, next->id);
        }
    }

    // Delete the expense
    free(expense);
    return;
}

// Select the previous node as the currently selected item.
void expense_list_select_previous_node(struct expense_list *expense_list) {
    if (expense_list == NULL) return;
    if (expense_list->head == NULL) return;

    struct expense_node* expense = expense_list->head;
    while (expense != NULL) {
        if (strcmp(expense->id, expense_list->id_currently_selected) == 0) {
            if (expense->prev != NULL) {
                strcpy(expense_list->id_currently_selected, expense->prev->id);
                return;
            }
            if (expense->prev == NULL) {
                struct expense_node* temp = expense;
                while (temp->next != NULL) {
                    temp = temp->next;
                }
                strcpy(expense_list->id_currently_selected, temp->id);
                return;
            }
        }
        expense = expense->next;
    }
}

// Select the next node as the currently selected item.
void expense_list_select_next_node(struct expense_list *expense_list) {
    if (expense_list == NULL) return;
    if (expense_list->head == NULL) return;

    struct expense_node* expense = expense_list->head;
    while (expense != NULL) {
        if (strcmp(expense->id, expense_list->id_currently_selected) == 0) {
            if (expense->next != NULL) {
                strcpy(expense_list->id_currently_selected, expense->next->id);
                return;
            }
            if (expense->next == NULL) {
                strcpy(expense_list->id_currently_selected,\
                expense_list->head->id);
                return;
            }
        }
        expense = expense->next;
    }
}

// Change the currently selected ID to the passed in ID in the expense list.
void expense_list_set_selected_id\
(struct expense_list *expense_list, char* id) {
    if (expense_list == NULL || id == NULL) return;
    if (expense_list->head == NULL) return;

    struct expense_node* expense = expense_list->head;
    while (expense != NULL) {
        if (strcmp(expense->id, id) == 0) {
            strcpy(expense_list->id_currently_selected, id);
        }
        expense = expense->next;
    }
}

// Get the expense type back as a string:
char* expense_list_get_node_type(struct expense_list* expense_list, char* id) {
    if (expense_list == NULL || id == NULL) return "";
    
    struct expense_node* expense = expense_list_get_node(expense_list, id);
    if (expense == NULL) return "";

    if (expense->type == expense_type_energy) return "Energy";
    if (expense->type == expense_type_rent) return "Rent";
    if (expense->type == expense_type_wage) return "Wage";
    if (expense->type == expense_type_misc) return "Misc";

    return "";
}


// Render the expense table GUI.
// This function displays a list of expenses as a table that can be selected.
// It is an overview.
enum program_status expense_table(struct nk_context* ctx,\
struct expense_list* expense_list) {
    if (ctx == NULL || expense_list == NULL) return program_status_running;

    // Button to return to enterprise menu.
    nk_layout_row_dynamic(ctx, ENTERPRISE_WIDGET_HEIGHT, 1);
    if (nk_button_label(ctx, "Return to Enterprise Menu")) {
        return program_status_enterprise_menu;
    }

    // Display the title.
    nk_label(ctx, "Expense Menu", NK_TEXT_CENTERED);

    // Create button to add new expenses to the table.
    if (nk_button_label(ctx, "New Expense")) {
        expense_list_append(expense_list);
    }

    // If there are no expenses, warn the user.
    if (expense_list->head == NULL) {
        nk_label(ctx, "No Expenses found.", NK_TEXT_CENTERED);
        return program_status_expense_table;
    }

    /* If there are expenses, go through the linked list of expenses.
    Copy the data from each expense and make it a button label.
    When the button is pressed, set the currently selected expense to that
    expense and switch to expense editor.*/
    struct expense_node* expense = expense_list->head;
    char* print_buffer = malloc(sizeof(char) * ENTERPRISE_STRING_LENGTH * 10);
    while (expense != NULL) {
        sprintf(print_buffer, \
        "ID: %s Type: %s Facility ID: %s Supplier ID: %s",expense->id,\
        expense_list_get_node_type(expense_list, expense->id),\
        expense->facility_id, expense->supplier_id);

        if (nk_button_label(ctx, print_buffer)) {
            strcpy(expense_list->id_currently_selected, expense->id);
            free(print_buffer);
            return program_status_expense_editor;
        }
        expense = expense->next;
    }
    free(print_buffer);
    return program_status_expense_table;
}

// Render the expense editor GUI.
// It gives the user an opportunity to edit a currently selected expense.
enum program_status expense_editor(struct nk_context* ctx,\
struct expense_list* expense_list) {
    if (ctx == NULL || expense_list == NULL)
        return program_status_enterprise_menu;

    // Display title and return to expense table button.
    nk_layout_row_dynamic(ctx, ENTERPRISE_WIDGET_HEIGHT, 1);
    if (nk_button_label(ctx, "Return to Expense Table")) {
        return program_status_expense_table;
    }
    nk_label(ctx, "Expense Editor", NK_TEXT_CENTERED);
    
    // Select currently selected expense.
    struct expense_node* expense = expense_list_get_node\
    (expense_list, expense_list->id_currently_selected);

    // If the currently selected expense does not exist:
    if (expense == NULL) {
        // Select the first expense instead.
        if (expense_list->head != NULL) {
            strcpy(expense_list->id_currently_selected, expense_list->head->id);
            return program_status_expense_editor;
        }

        // Else tell the user to create a new expense 
        // if the first expense doesn't exist.
        else {
            nk_label(ctx, "No Expenses found.", NK_TEXT_CENTERED);
            if (nk_button_label(ctx, "New Expense")) {
                expense_list_append(expense_list);
            }
            return program_status_expense_editor;
        }
    }

    // Display edit fields to edit expense entries.
    nk_layout_row_template_begin(ctx, ENTERPRISE_WIDGET_HEIGHT);
    nk_layout_row_template_push_static(ctx, 150);
    nk_layout_row_template_push_dynamic(ctx);
    nk_layout_row_template_end(ctx);

    // Expense type editor.
    int type = 0;
    if (expense->type == expense_type_rent) type = 0;
    if (expense->type == expense_type_wage) type = 1;
    if (expense->type == expense_type_energy) type = 2;
    if (expense->type == expense_type_misc) type = 3;

    const char* expense_types[] = {"Rent", "Wage", "Energy", "Misc"};
    nk_label(ctx, "Type: ", NK_TEXT_LEFT);
    type = nk_combo(ctx, expense_types, NK_LEN(expense_types), type, \
    ENTERPRISE_WIDGET_HEIGHT, nk_vec2(WINDOW_WIDTH, 200));

    if (type == 0) expense->type = expense_type_rent;
    if (type == 1) expense->type = expense_type_wage;
    if (type == 3) expense->type = expense_type_energy;
    if (type == 4) expense->type = expense_type_misc;

    nk_label(ctx, "ID: ", NK_TEXT_LEFT);
    nk_edit_string_zero_terminated(ctx, NK_EDIT_READ_ONLY, \
    expense->id, ENTERPRISE_STRING_LENGTH, nk_filter_default);

    nk_label(ctx, "Facility ID: ", NK_TEXT_LEFT);
    nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, \
    expense->facility_id, ENTERPRISE_STRING_LENGTH, nk_filter_default);

    nk_label(ctx, "Supplier ID: ", NK_TEXT_LEFT);
    nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, \
    expense->supplier_id, ENTERPRISE_STRING_LENGTH, nk_filter_default);

    // Move between next and previous expenses.
    nk_layout_row_dynamic(ctx, ENTERPRISE_WIDGET_HEIGHT, 2);
    if (nk_button_symbol_label\
    (ctx, NK_SYMBOL_TRIANGLE_LEFT, "prev", NK_TEXT_RIGHT)) {
        expense_list_select_previous_node(expense_list);
    }

    if (nk_button_symbol_label\
    (ctx, NK_SYMBOL_TRIANGLE_RIGHT, "next", NK_TEXT_LEFT)) {
        expense_list_select_next_node(expense_list);
    }

    nk_layout_row_dynamic(ctx, ENTERPRISE_WIDGET_HEIGHT, 1);
    // Create new expense button.
    if (nk_button_label(ctx, "New Expense")) {
        expense_list_append(expense_list);
    }

    // Create deletion button.
    // This will ask the user to confirm whether they want to delete the node.
    if (nk_button_label(ctx, "Delete Expense")) {
        expense_list->deletion_requested = true;
    }

    if (expense_list->deletion_requested == true) {
        nk_label(ctx, "Confirm deletion?", NK_TEXT_CENTERED);
        nk_layout_row_dynamic(ctx, ENTERPRISE_WIDGET_HEIGHT, 2);

        if (nk_button_label(ctx, "Yes")) {
            expense_list_delete_node(expense_list, expense->id);
            expense_list->deletion_requested = false;
        }
        if (nk_button_label(ctx, "No")) {
            expense_list->deletion_requested = false;
        }        
    }
    
    return program_status_expense_editor;
}