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

#include "employee_facilities.c"

/* How employees work.
employees are stored in a struct that contains a pointer to the head of a
linked list containing all the employees. The struct that stores the linked
list is directly added to the enterprise struct and stores metadata about the
list. All employees are assigned a unique ID, no two employees can have the
same ID, the employee_list structure keeps track of that.

Data structures:
employee_node: An individual employee.
employee_list: A structure holding important metadata about the employee
linked list.

employee list metadata:
employee_list->id_last_assigned: This is incremented by 1 every time a new
employee is added in order to ensure no two employees ever have the same ID.

employee_list->id_currently_selected: This is the ID that is selected in the
employee editor dialogue.
*/

struct employee_node {
    char id[ENTERPRISE_STRING_LENGTH];
    char name[ENTERPRISE_STRING_LENGTH];
    char email[ENTERPRISE_STRING_LENGTH];
    char phone[ENTERPRISE_STRING_LENGTH];
    char address[ENTERPRISE_STRING_LENGTH];

    struct employee_facility_list* employee_facility_list;
    struct employee_node* prev;
    struct employee_node* next;
};

// employee node constructor and initialiser.
// Returns employee node on success, or NULL on failure.
struct employee_node *employee_node_new() {
    struct employee_node* employee = malloc(sizeof(struct employee_node));
    if (employee == NULL) return NULL;
    strcpy(employee->id, "");
    strcpy(employee->name, "");
    strcpy(employee->email, "");
    strcpy(employee->phone, "");
    strcpy(employee->address, "");

    employee->employee_facility_list = NULL;

    employee->prev = NULL;
    employee->next = NULL;

    return employee;
}

// Free everything associated with an employee.
void employee_node_free(struct employee_node* employee) {
    if (employee == NULL) return;
    if (employee->employee_facility_list != NULL) {
        employee_facility_list_free(employee->employee_facility_list);
    }
    free(employee);
}

// employee list metadata structure.
struct employee_list {
    struct employee_node *head;
    char id_last_assigned[ENTERPRISE_STRING_LENGTH];
    char id_currently_selected[ENTERPRISE_STRING_LENGTH];
    bool deletion_requested;
};

// employee list constructor.
// Returns employee list on success, or NULL on failure.
struct employee_list* employee_list_new() {
    struct employee_list* employee_list = malloc(sizeof(struct employee_list));
    if (employee_list == NULL) return NULL;
    employee_list->head = NULL;
    strcpy(employee_list->id_last_assigned, "0");
    strcpy(employee_list->id_currently_selected, "0");
    employee_list->deletion_requested = false;
    return employee_list;
}



// Free an entire employee linked list
void employee_node_linked_list_free(struct employee_node* employee) {
    if (employee == NULL) return;
    struct employee_node* next = NULL;
    while (employee != NULL) {
        next = employee->next;
        employee_node_free(employee);
        employee = next;
    }
}

// Free all memory associated with a employee list.
void employee_list_free(struct employee_list* employee_list) {
    if (employee_list == NULL) return;

    if (employee_list->head != NULL) {
        employee_node_linked_list_free(employee_list->head);
    }

    free(employee_list);
    return;
}

// Append a new employee to a employee list.
void employee_list_append(struct employee_list* employee_list) {
    if (employee_list == NULL) return;

    // Increment unique ID by 1.
    char buffer[ENTERPRISE_STRING_LENGTH];
    sprintf(buffer, "%lld", atoll(employee_list->id_last_assigned) + 1);
    strcpy(employee_list->id_last_assigned, buffer);

    // Add new node to start of employee list if employee linked list does
    // not exist.
    if (employee_list->head == NULL) {
        employee_list->head = employee_node_new();
        strcpy(employee_list->head->id, employee_list->id_last_assigned);
        strcpy(employee_list->id_currently_selected,
        employee_list->id_last_assigned);
        return;
    }

    // Else add the new node to the end of the list.
    struct employee_node* employee = employee_list->head;
    while (employee->next != NULL) {
        employee = employee->next;
    }
    
    employee->next = employee_node_new();
    employee->next->prev = employee;

    strcpy(employee->next->id, employee_list->id_last_assigned);
    strcpy(employee_list->id_currently_selected,
    employee_list->id_last_assigned);

    return;
}

// Get the number of employee nodes in the employee list
int employee_list_get_num_employee_nodes(struct employee_list* employee_list) {
    if (employee_list == NULL) return 0;
    if (employee_list->head == NULL) return 0;

    struct employee_node* employee = employee_list->head;
    int employee_count = 0;
    while (employee != NULL) {
        employee_count++;
        employee = employee->next;
    }
    return employee_count;
}

// Return a pointer to a employee node according to ID.
// Returns NULL on failure.
struct employee_node *employee_list_get_node\
(struct employee_list *employee_list, char *id) {
    if (employee_list == NULL || id == NULL) return NULL;
    if (employee_list->head == NULL) return NULL;

    struct employee_node* employee = employee_list->head;
    while (employee != NULL) {
        if (strcmp(employee->id, id) == 0) return employee;
        employee = employee->next;
    }
    return NULL;
}

// Searches for a employee by ID and deletes it
void employee_list_delete_node(struct employee_list *employee_list, char *id) {
    if (employee_list == NULL || id == NULL) return;
    if (employee_list->head == NULL) return;

    // Delete the head if it is the ID that is requested to be deleted.
    if (strcmp(id, employee_list->head->id) == 0) {
        if (employee_list->head->next == NULL) {
            employee_node_free(employee_list->head);
            employee_list->head = NULL;
            return;
        }

        struct employee_node *next = employee_list->head->next;
        employee_node_free(employee_list->head);
        employee_list->head = next;
        return;
    }

    // Else search for the employee to delete.
    struct employee_node* employee = employee_list->head;
    while (employee != NULL) {
        if (strcmp(employee->id, id) == 0) break;
        employee = employee->next;
    }
    if (employee == NULL) return;

    // Fix the pointers of the next and previous nodes to the employee to be
    // deleted in order to ensure that traversal still works.

    struct employee_node* prev = employee->prev;
    struct employee_node* next = employee->next;

    if (prev != NULL) {
        if (next == NULL) {
            prev->next = NULL;
            strcpy(employee_list->id_currently_selected, prev->id);
        }
        if (next != NULL) {
            prev->next = next;
            next->prev = prev;
            strcpy(employee_list->id_currently_selected, next->id);
        }
    }

    if (prev == NULL) {
        if (next != NULL) {
            next->prev = NULL;
            strcpy(employee_list->id_currently_selected, next->id);
        }
    }

    // Delete the employee
    employee_node_free(employee);
    return;
}

// Select the previous node as the currently selected item.
void employee_list_select_previous_node(struct employee_list *employee_list) {
    if (employee_list == NULL) return;
    if (employee_list->head == NULL) return;

    struct employee_node* employee = employee_list->head;
    while (employee != NULL) {
        if (strcmp(employee->id, employee_list->id_currently_selected) == 0) {
            if (employee->prev != NULL) {
                strcpy(employee_list->id_currently_selected, employee->prev->id);
                return;
            }
            if (employee->prev == NULL) {
                struct employee_node* temp = employee;
                while (temp->next != NULL) {
                    temp = temp->next;
                }
                strcpy(employee_list->id_currently_selected, temp->id);
                return;
            }
        }
        employee = employee->next;
    }
}

// Select the next node as the currently selected item.
void employee_list_select_next_node(struct employee_list *employee_list) {
    if (employee_list == NULL) return;
    if (employee_list->head == NULL) return;

    struct employee_node* employee = employee_list->head;
    while (employee != NULL) {
        if (strcmp(employee->id, employee_list->id_currently_selected) == 0) {
            if (employee->next != NULL) {
                strcpy(employee_list->id_currently_selected, employee->next->id);
                return;
            }
            if (employee->next == NULL) {
                strcpy(employee_list->id_currently_selected,\
                employee_list->head->id);
                return;
            }
        }
        employee = employee->next;
    }
}

// Change the currently selected ID to the passed in ID in the employee list.
void employee_list_set_selected_id\
(struct employee_list *employee_list, char* id) {
    if (employee_list == NULL || id == NULL) return;
    if (employee_list->head == NULL) return;

    struct employee_node* employee = employee_list->head;
    while (employee != NULL) {
        if (strcmp(employee->id, id) == 0) {
            strcpy(employee_list->id_currently_selected, id);
        }
        employee = employee->next;
    }
}

// Render the employee table GUI.
// This function displays a list of employees as a table that can be selected.
// It is an overview.
enum program_status employee_table(struct nk_context* ctx,\
struct employee_list* employee_list) {
    if (ctx == NULL || employee_list == NULL) return program_status_running;

    // Button to return to enterprise menu.
    nk_layout_row_dynamic(ctx, ENTERPRISE_WIDGET_HEIGHT, 1);
    if (nk_button_label(ctx, "Return to Enterprise Menu")) {
        return program_status_enterprise_menu;
    }

    // Display the title.
    nk_label(ctx, "Employee Menu", NK_TEXT_CENTERED);

    // Create button to add new employees to the table.
    if (nk_button_label(ctx, "New Employee")) {
        employee_list_append(employee_list);
    }

    // If there are no employees, warn the user.
    if (employee_list->head == NULL) {
        nk_label(ctx, "No Employees found.", NK_TEXT_CENTERED);
        return program_status_employee_table;
    }

    /* If there are employees, go through the linked list of employees.
    Copy the data from each employee and make it a button label.
    When the button is pressed, set the currently selected employee to that
    employee and switch to employee editor.*/
    struct employee_node* employee = employee_list->head;
    char* print_buffer = malloc(sizeof(char) * ENTERPRISE_STRING_LENGTH * 10);
    while (employee != NULL) {
        sprintf(print_buffer, \
        "ID: %s Name: %s Email: %s Phone: %s Address: %s",employee->id,\
        employee->name, employee->email, employee->phone, employee->address);

        if (nk_button_label(ctx, print_buffer)) {
            strcpy(employee_list->id_currently_selected, employee->id);
            free(print_buffer);
            return program_status_employee_editor;
        }
        employee = employee->next;
    }
    free(print_buffer);
    return program_status_employee_table;
}

// Render the employee editor GUI.
// It gives the user an opportunity to edit a currently selected employee.
enum program_status employee_editor(struct nk_context* ctx,\
struct employee_list* employee_list) {
    if (ctx == NULL || employee_list == NULL)
        return program_status_enterprise_menu;

    // Display title and return to employee table button.
    nk_layout_row_dynamic(ctx, ENTERPRISE_WIDGET_HEIGHT, 1);
    if (nk_button_label(ctx, "Return to Employee Table")) {
        return program_status_employee_table;
    }
    nk_label(ctx, "Employee Editor", NK_TEXT_CENTERED);
    
    // Select currently selected employee.
    struct employee_node* employee = employee_list_get_node\
    (employee_list, employee_list->id_currently_selected);

    // If the currently selected employee does not exist:
    if (employee == NULL) {
        // Select the first employee instead.
        if (employee_list->head != NULL) {
            strcpy(employee_list->id_currently_selected, employee_list->head->id);
            return program_status_employee_editor;
        }

        // Else tell the user to create a new employee 
        // if the first employee doesn't exist.
        else {
            nk_label(ctx, "No Employees found.", NK_TEXT_CENTERED);
            if (nk_button_label(ctx, "New Employee")) {
                employee_list_append(employee_list);
            }
            return program_status_employee_editor;
        }
    }

    // Display edit fields to edit employee entries.
    nk_layout_row_template_begin(ctx, ENTERPRISE_WIDGET_HEIGHT);
    nk_layout_row_template_push_static(ctx, 150);
    nk_layout_row_template_push_dynamic(ctx);
    nk_layout_row_template_end(ctx);

    nk_label(ctx, "ID: ", NK_TEXT_LEFT);
    nk_edit_string_zero_terminated(ctx, NK_EDIT_READ_ONLY, \
    employee->id, ENTERPRISE_STRING_LENGTH, nk_filter_default);

    nk_label(ctx, "Name: ", NK_TEXT_LEFT);
    nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, \
    employee->name, ENTERPRISE_STRING_LENGTH, nk_filter_default);

    nk_label(ctx, "Phone: ", NK_TEXT_LEFT);
    nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, \
    employee->phone, ENTERPRISE_STRING_LENGTH, nk_filter_default);

    nk_label(ctx, "Email: ", NK_TEXT_LEFT);
    nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, \
    employee->email, ENTERPRISE_STRING_LENGTH, nk_filter_default);

    nk_label(ctx, "Address: ", NK_TEXT_LEFT);
    nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, \
    employee->address, ENTERPRISE_STRING_LENGTH, nk_filter_default);

    nk_layout_row_dynamic(ctx, ENTERPRISE_WIDGET_HEIGHT, 1);
    if (nk_button_label(ctx, "Facilities")) {
        if (employee->employee_facility_list == NULL) {
            employee->employee_facility_list = employee_facility_list_new();
        }
        return program_status_employee_facility_table;
    }

    // Move between next and previous employees.
    nk_layout_row_dynamic(ctx, ENTERPRISE_WIDGET_HEIGHT, 2);
    if (nk_button_symbol_label\
    (ctx, NK_SYMBOL_TRIANGLE_LEFT, "prev", NK_TEXT_RIGHT)) {
        employee_list_select_previous_node(employee_list);
    }

    if (nk_button_symbol_label\
    (ctx, NK_SYMBOL_TRIANGLE_RIGHT, "next", NK_TEXT_LEFT)) {
        employee_list_select_next_node(employee_list);
    }

    nk_layout_row_dynamic(ctx, ENTERPRISE_WIDGET_HEIGHT, 1);
    // Create new employee button.
    if (nk_button_label(ctx, "New Employee")) {
        employee_list_append(employee_list);
    }

    // Create deletion button.
    // This will ask the user to confirm whether they want to delete the node.
    if (nk_button_label(ctx, "Delete Employee")) {
        employee_list->deletion_requested = true;
    }

    if (employee_list->deletion_requested == true) {
        nk_label(ctx, "Confirm deletion?", NK_TEXT_CENTERED);
        nk_layout_row_dynamic(ctx, ENTERPRISE_WIDGET_HEIGHT, 2);

        if (nk_button_label(ctx, "Yes")) {
            employee_list_delete_node(employee_list, employee->id);
            employee_list->deletion_requested = false;
        }
        if (nk_button_label(ctx, "No")) {
            employee_list->deletion_requested = false;
        }        
    }
    
    return program_status_employee_editor;
}