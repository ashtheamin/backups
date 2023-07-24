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
#include "employees.c"
#include "inventory.c"
#include "customers.c"
#include "suppliers.c"
#include "expenses.c"
#include "orders.c"
#endif

/* How employee_facilitys work.
employee_facilitys are stored in a struct that contains a pointer to the head of a
linked list containing all the employee_facilitys. The struct that stores the linked
list is directly added to the enterprise struct and stores metadata about the
list. All employee_facilitys are assigned a unique ID, no two employee_facilitys can have the
same ID, the employee_facility_list structure keeps track of that.

Data structures:
employee_facility_node: An individual employee_facility.
employee_facility_list: A structure holding important metadata about the employee_facility
linked list.

employee_facility list metadata:
employee_facility_list->id_last_assigned: This is incremented by 1 every time a new
employee_facility is added in order to ensure no two employee_facilitys ever have the same ID.

employee_facility_list->id_currently_selected: This is the ID that is selected in the
employee_facility editor dialogue.
*/

struct employee_facility_node {
    char id[ENTERPRISE_STRING_LENGTH];
    char facility_id[ENTERPRISE_STRING_LENGTH];

    struct employee_facility_node* prev;
    struct employee_facility_node* next;
};

// employee_facility node constructor and initialiser.
// Returns employee_facility node on success, or NULL on failure.
struct employee_facility_node *employee_facility_node_new() {
    struct employee_facility_node* employee_facility = malloc(sizeof(struct employee_facility_node));
    if (employee_facility == NULL) return NULL;
    strcpy(employee_facility->id, "");
    strcpy(employee_facility->facility_id, "");

    employee_facility->prev = NULL;
    employee_facility->next = NULL;

    return employee_facility;
}

// employee_facility list metadata structure.
struct employee_facility_list {
    struct employee_facility_node *head;
    char id_last_assigned[ENTERPRISE_STRING_LENGTH];
    char id_currently_selected[ENTERPRISE_STRING_LENGTH];
    bool deletion_requested;
    bool addition_requested;
};

// employee_facility list constructor.
// Returns employee_facility list on success, or NULL on failure.
struct employee_facility_list* employee_facility_list_new() {
    struct employee_facility_list* employee_facility_list = malloc(sizeof(struct employee_facility_list));
    if (employee_facility_list == NULL) return NULL;
    employee_facility_list->head = NULL;
    strcpy(employee_facility_list->id_last_assigned, "0");
    strcpy(employee_facility_list->id_currently_selected, "0");
    employee_facility_list->deletion_requested = false;
    employee_facility_list->addition_requested = false;
    return employee_facility_list;
}

// Free an entire employee_facility linked list
void employee_facility_node_linked_list_free(struct employee_facility_node* employee_facility) {
    if (employee_facility == NULL) return;
    struct employee_facility_node* next = NULL;
    while (employee_facility != NULL) {
        next = employee_facility->next;
        free(employee_facility);
        employee_facility = next;
    }
}

// Free all memory associated with a employee_facility list.
void employee_facility_list_free(struct employee_facility_list* employee_facility_list) {
    if (employee_facility_list == NULL) return;

    if (employee_facility_list->head != NULL) {
        employee_facility_node_linked_list_free(employee_facility_list->head);
    }

    free(employee_facility_list);
    return;
}

// Append a new employee_facility to a employee_facility list.
void employee_facility_list_append(struct employee_facility_list* employee_facility_list) {
    if (employee_facility_list == NULL) return;

    // Increment unique ID by 1.
    char buffer[ENTERPRISE_STRING_LENGTH];
    sprintf(buffer, "%lld", atoll(employee_facility_list->id_last_assigned) + 1);
    strcpy(employee_facility_list->id_last_assigned, buffer);

    // Add new node to start of employee_facility list if employee_facility linked list does
    // not exist.
    if (employee_facility_list->head == NULL) {
        employee_facility_list->head = employee_facility_node_new();
        strcpy(employee_facility_list->head->id, employee_facility_list->id_last_assigned);
        strcpy(employee_facility_list->id_currently_selected,
        employee_facility_list->id_last_assigned);
        return;
    }

    // Else add the new node to the end of the list.
    struct employee_facility_node* employee_facility = employee_facility_list->head;
    while (employee_facility->next != NULL) {
        employee_facility = employee_facility->next;
    }
    
    employee_facility->next = employee_facility_node_new();
    employee_facility->next->prev = employee_facility;

    strcpy(employee_facility->next->id, employee_facility_list->id_last_assigned);
    strcpy(employee_facility_list->id_currently_selected,
    employee_facility_list->id_last_assigned);

    return;
}

// Get the number of employee_facility nodes in the employee_facility list
int employee_facility_list_get_num_employee_facility_nodes(struct employee_facility_list* employee_facility_list) {
    if (employee_facility_list == NULL) return 0;
    if (employee_facility_list->head == NULL) return 0;

    struct employee_facility_node* employee_facility = employee_facility_list->head;
    int employee_facility_count = 0;
    while (employee_facility != NULL) {
        employee_facility_count++;
        employee_facility = employee_facility->next;
    }
    return employee_facility_count;
}

// Return a pointer to a employee_facility node according to ID.
// Returns NULL on failure.
struct employee_facility_node *employee_facility_list_get_node\
(struct employee_facility_list *employee_facility_list, char *id) {
    if (employee_facility_list == NULL || id == NULL) return NULL;
    if (employee_facility_list->head == NULL) return NULL;

    struct employee_facility_node* employee_facility = employee_facility_list->head;
    while (employee_facility != NULL) {
        if (strcmp(employee_facility->id, id) == 0) return employee_facility;
        employee_facility = employee_facility->next;
    }
    return NULL;
}

// Return a pointer to a employee_facility node according to facility ID.
// Returns NULL on failure.
struct employee_facility_node *employee_facility_list_get_node_by_facility_id\
(struct employee_facility_list *employee_facility_list, char *id) {
    if (employee_facility_list == NULL || id == NULL) return NULL;
    if (employee_facility_list->head == NULL) return NULL;

    struct employee_facility_node* employee_facility = employee_facility_list->head;
    while (employee_facility != NULL) {
        if (strcmp(employee_facility->facility_id, id) == 0) return employee_facility;
        employee_facility = employee_facility->next;
    }
    return NULL;
}

// Searches for a employee_facility by ID and deletes it
void employee_facility_list_delete_node(struct employee_facility_list *employee_facility_list, char *id) {
    if (employee_facility_list == NULL || id == NULL) return;
    if (employee_facility_list->head == NULL) return;

    // Delete the head if it is the ID that is requested to be deleted.
    if (strcmp(id, employee_facility_list->head->id) == 0) {
        if (employee_facility_list->head->next == NULL) {
            free(employee_facility_list->head);
            employee_facility_list->head = NULL;
            return;
        }

        struct employee_facility_node *next = employee_facility_list->head->next;
        free(employee_facility_list->head);
        employee_facility_list->head = next;
        return;
    }

    // Else search for the employee_facility to delete.
    struct employee_facility_node* employee_facility = employee_facility_list->head;
    while (employee_facility != NULL) {
        if (strcmp(employee_facility->id, id) == 0) break;
        employee_facility = employee_facility->next;
    }
    if (employee_facility == NULL) return;

    // Fix the pointers of the next and previous nodes to the employee_facility to be
    // deleted in order to ensure that traversal still works.

    struct employee_facility_node* prev = employee_facility->prev;
    struct employee_facility_node* next = employee_facility->next;

    if (prev != NULL) {
        if (next == NULL) {
            prev->next = NULL;
            strcpy(employee_facility_list->id_currently_selected, prev->id);
        }
        if (next != NULL) {
            prev->next = next;
            next->prev = prev;
            strcpy(employee_facility_list->id_currently_selected, next->id);
        }
    }

    if (prev == NULL) {
        if (next != NULL) {
            next->prev = NULL;
            strcpy(employee_facility_list->id_currently_selected, next->id);
        }
    }

    // Delete the employee_facility
    free(employee_facility);
    return;
}

// Select the previous node as the currently selected item.
void employee_facility_list_select_previous_node(struct employee_facility_list *employee_facility_list) {
    if (employee_facility_list == NULL) return;
    if (employee_facility_list->head == NULL) return;

    struct employee_facility_node* employee_facility = employee_facility_list->head;
    while (employee_facility != NULL) {
        if (strcmp(employee_facility->id, employee_facility_list->id_currently_selected) == 0) {
            if (employee_facility->prev != NULL) {
                strcpy(employee_facility_list->id_currently_selected, employee_facility->prev->id);
                return;
            }
            if (employee_facility->prev == NULL) {
                struct employee_facility_node* temp = employee_facility;
                while (temp->next != NULL) {
                    temp = temp->next;
                }
                strcpy(employee_facility_list->id_currently_selected, temp->id);
                return;
            }
        }
        employee_facility = employee_facility->next;
    }
}

// Select the next node as the currently selected item.
void employee_facility_list_select_next_node(struct employee_facility_list *employee_facility_list) {
    if (employee_facility_list == NULL) return;
    if (employee_facility_list->head == NULL) return;

    struct employee_facility_node* employee_facility = employee_facility_list->head;
    while (employee_facility != NULL) {
        if (strcmp(employee_facility->id, employee_facility_list->id_currently_selected) == 0) {
            if (employee_facility->next != NULL) {
                strcpy(employee_facility_list->id_currently_selected, employee_facility->next->id);
                return;
            }
            if (employee_facility->next == NULL) {
                strcpy(employee_facility_list->id_currently_selected,\
                employee_facility_list->head->id);
                return;
            }
        }
        employee_facility = employee_facility->next;
    }
}

// Change the currently selected ID to the passed in ID in the employee_facility list.
void employee_facility_list_set_selected_id\
(struct employee_facility_list *employee_facility_list, char* id) {
    if (employee_facility_list == NULL || id == NULL) return;
    if (employee_facility_list->head == NULL) return;

    struct employee_facility_node* employee_facility = employee_facility_list->head;
    while (employee_facility != NULL) {
        if (strcmp(employee_facility->id, id) == 0) {
            strcpy(employee_facility_list->id_currently_selected, id);
        }
        employee_facility = employee_facility->next;
    }
}

// Render the employee facilities table GUI.
// This is an overview of the facilities that an employee works at.
enum program_status employee_facility_table(struct nk_context* ctx,\
struct employee_facility_list* employee_facility_list, \
struct facility_list* facility_list) {
    if (ctx == NULL || employee_facility_list == NULL)\
    return program_status_employee_editor;

    // Button to return to employee editor.
    nk_layout_row_dynamic(ctx, ENTERPRISE_WIDGET_HEIGHT, 1);
    if (nk_button_label(ctx, "Return to Employee Editor")) {
        return program_status_employee_editor;
    }

    // Handle case where there are no facilities.
    if (facility_list->head == NULL) {
        nk_label(ctx, "There are no facilities available to add employee to.",\
        NK_TEXT_CENTERED);
        if (nk_button_label(ctx, "Go to facility table.")) {
            return program_status_facility_table;
        }
    }

    else {
        // Display the title.
        nk_label(ctx, "Employee Facilities Menu", NK_TEXT_CENTERED);
        nk_label(ctx, "Facilities employee currently works at: ", NK_TEXT_CENTERED);

        // Present all facilities that the employee works for.
        struct employee_facility_node* employee_facility\
        = employee_facility_list->head;
        char* print_buffer = malloc(sizeof(char) * ENTERPRISE_STRING_LENGTH * 10);
        while (employee_facility != NULL) {

            struct facility_node* facility = facility_list_get_node(\
            facility_list, employee_facility->id);
            if (facility == NULL) break;

            sprintf(print_buffer, "Facility ID: %s Name: %s", \
            employee_facility->id, facility->name);

            if (nk_button_label(ctx, print_buffer)) {
                strcpy(employee_facility_list->id_currently_selected,\
                employee_facility->id);
                free(print_buffer);
                return program_status_employee_facility_editor;
            }
            employee_facility = employee_facility->next;
        }
        free(print_buffer);

        // Create button to add new employee facilities to the table.
        nk_layout_row_dynamic(ctx, ENTERPRISE_WIDGET_HEIGHT, 1);
        if (nk_button_label(ctx, "Add Facility")) {
            if (employee_facility_list_get_num_employee_facility_nodes\
            (employee_facility_list) != facility_list_get_num_facility_nodes(\
            facility_list)) {
            employee_facility_list->addition_requested = true;
        }
        }

        /* If we want to add a new facility.
        Search through all the facilities in the facility table, display the 
        facilities that aren't currently worked for by the employee, and 
        if clicked on, add it to the list of facilities the current employee 
        works for. */
        if (employee_facility_list->addition_requested == true) {
            print_buffer = malloc(sizeof(char) * ENTERPRISE_STRING_LENGTH * 10);
            struct facility_node* facility = facility_list->head;

            while (facility != NULL) {
                bool show = false;
                sprintf(print_buffer, "ID : %s Facility Name: %s",\
                facility->id, facility->name);
                if (employee_facility_list_get_node_by_facility_id(\
                employee_facility_list,facility->id) == NULL) show = true;
                    if (show == true) {
                        if (nk_button_label(ctx, print_buffer)) {
                            employee_facility_list_append(employee_facility_list);

                            strcpy(employee_facility_list_get_node\
                            (employee_facility_list,\
                            employee_facility_list->id_currently_selected)\
                            ->facility_id, facility->id);
                            employee_facility_list->addition_requested = false;
                        }
                    }
                facility = facility->next;
            }
            free(print_buffer);
        }
    }
    return program_status_employee_facility_table;
}

// Render the employee_facility editor GUI.
// It gives the user an opportunity to edit a currently selected employee_facility.
enum program_status employee_facility_editor(struct nk_context* ctx,\
struct employee_facility_list* employee_facility_list, \
struct facility_list* facility_list) {
    if (ctx == NULL || employee_facility_list == NULL)
        return program_status_enterprise_menu;

    // Display title and return to employee_facility table button.
    nk_layout_row_dynamic(ctx, ENTERPRISE_WIDGET_HEIGHT, 1);
    if (nk_button_label(ctx, "Return to Employee Facility Table")) {
        return program_status_employee_facility_table;
    }
    nk_label(ctx, "Employee Facility Editor", NK_TEXT_CENTERED);
    
    // Select currently selected employee_facility.
    struct employee_facility_node* employee_facility = \
    employee_facility_list_get_node(employee_facility_list,\
    employee_facility_list->id_currently_selected);

    // If the currently selected employee_facility does not exist:
    if (employee_facility == NULL) {
        // Select the first employee_facility instead.
        if (employee_facility_list->head != NULL) {
            strcpy(employee_facility_list->id_currently_selected, \
            employee_facility_list->head->id);
            return program_status_employee_facility_editor;
        }

        // Else tell the user to create a new employee_facility 
        // if the first employee_facility doesn't exist.
        else {
            nk_label(ctx, "No facilities found.", NK_TEXT_CENTERED);
            if (nk_button_label(ctx, "New Facility")) {
                employee_facility_list->addition_requested = true;

            }
            return program_status_employee_facility_editor;
        }
    }

    // Display edit fields to edit employee_facility entries.
    nk_layout_row_template_begin(ctx, ENTERPRISE_WIDGET_HEIGHT);
    nk_layout_row_template_push_static(ctx, 150);
    nk_layout_row_template_push_dynamic(ctx);
    nk_layout_row_template_end(ctx);

    nk_label(ctx, "Facility ID: ", NK_TEXT_LEFT);
    nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, \
    employee_facility->facility_id, ENTERPRISE_STRING_LENGTH, nk_filter_default);

    // Move between next and previous employee_facilitys.
    nk_layout_row_dynamic(ctx, ENTERPRISE_WIDGET_HEIGHT, 2);
    if (nk_button_symbol_label\
    (ctx, NK_SYMBOL_TRIANGLE_LEFT, "prev", NK_TEXT_RIGHT)) {
        employee_facility_list_select_previous_node(employee_facility_list);
    }

    if (nk_button_symbol_label\
    (ctx, NK_SYMBOL_TRIANGLE_RIGHT, "next", NK_TEXT_LEFT)) {
        employee_facility_list_select_next_node(employee_facility_list);
    }

    nk_layout_row_dynamic(ctx, ENTERPRISE_WIDGET_HEIGHT, 1);

    // Create deletion button.
    // This will ask the user to confirm whether they want to delete the node.
    if (nk_button_label(ctx, "Delete employee facility")) {
        employee_facility_list->deletion_requested = true;
    }

    if (employee_facility_list->deletion_requested == true) {
        nk_label(ctx, "Confirm deletion?", NK_TEXT_CENTERED);
        nk_layout_row_dynamic(ctx, ENTERPRISE_WIDGET_HEIGHT, 2);

        if (nk_button_label(ctx, "Yes")) {
            employee_facility_list_delete_node(employee_facility_list, employee_facility->id);
            employee_facility_list->deletion_requested = false;
        }
        if (nk_button_label(ctx, "No")) {
            employee_facility_list->deletion_requested = false;
        }        
    }

     // Create button to add new employee facilities to the table.
    if (nk_button_label(ctx, "Add Facility")) {
        if (employee_facility_list_get_num_employee_facility_nodes\
        (employee_facility_list) != facility_list_get_num_facility_nodes(\
        facility_list)) {
            employee_facility_list->addition_requested = true;
        }
    }

    if (employee_facility_list->addition_requested == true) {
            char* print_buffer = malloc(sizeof(char) * ENTERPRISE_STRING_LENGTH * 10);
            struct facility_node* facility = facility_list->head;

            while (facility != NULL) {
                bool show = false;
                sprintf(print_buffer, "ID : %s Facility Name: %s",\
                facility->id, facility->name);
                if (employee_facility_list_get_node_by_facility_id(\
                employee_facility_list,facility->id) == NULL) show = true;
                    if (show == true) {
                        if (nk_button_label(ctx, print_buffer)) {
                            employee_facility_list_append(employee_facility_list);

                            strcpy(employee_facility_list_get_node\
                            (employee_facility_list,\
                            employee_facility_list->id_currently_selected)\
                            ->facility_id, facility->id);
                            employee_facility_list->addition_requested = false;
                        }
                    }
                facility = facility->next;
            }
            free(print_buffer);
            
        }
    
    return program_status_employee_facility_editor;
}