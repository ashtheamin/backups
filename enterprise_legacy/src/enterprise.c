
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

#ifndef PROGRAM_STATES
#define PROGRAM_STATES
#include "program_states.c"
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

/* What does this file do?
It provides the enterprise data structure, which holds all the data related to
the enterprise. It stores lists of everything needed in an enterprise.
*/

// The enterprise struct.
// This holds all relevant database information about the enterprise.
struct enterprise {
    char name[ENTERPRISE_STRING_LENGTH];
    char balance[ENTERPRISE_STRING_LENGTH];
    struct facility_list* facility_list;
    struct employee_list* employee_list;
    struct item_list* item_list;
    struct customer_list* customer_list;
    struct supplier_list* supplier_list;
    struct expense_list* expense_list;
    struct order_list* order_list;
};

// Enterprise instance constructor.
// Returns pointer to enterprise on success, else returns NULL.
struct enterprise* enterprise_new() {
    struct enterprise* enterprise = malloc(sizeof(struct enterprise));
    if (enterprise == NULL) return NULL;

    strcpy(enterprise->name, "");
    strcpy(enterprise->balance, "");
    enterprise->facility_list = facility_list_new();
    enterprise->employee_list = employee_list_new();
    enterprise->item_list = item_list_new();
    enterprise->customer_list = customer_list_new();
    enterprise->supplier_list = supplier_list_new();
    enterprise->expense_list = expense_list_new();
    enterprise->order_list = order_list_new();
    return enterprise;
}

// Frees memory associated with enterprise.
void enterprise_quit(struct enterprise* enterprise) {
    if (enterprise == NULL) return;
    if (enterprise->facility_list != NULL) 
        {facility_list_free(enterprise->facility_list);}
    if (enterprise->employee_list != NULL) 
        {employee_list_free(enterprise->employee_list);}
    if (enterprise->item_list != NULL) 
        {item_list_free(enterprise->item_list);}
    if (enterprise->customer_list != NULL) 
        {customer_list_free(enterprise->customer_list);}
    if (enterprise->supplier_list != NULL) 
        {supplier_list_free(enterprise->supplier_list);}
    if (enterprise->expense_list != NULL) 
        {expense_list_free(enterprise->expense_list);}
    if (enterprise->order_list != NULL) 
        {order_list_free(enterprise->order_list);}
    free(enterprise);
    return;
}

// Render the enterprise menu GUI.
enum program_status enterprise_menu\
(struct nk_context* ctx, struct enterprise* enterprise) {
    if (ctx == NULL || enterprise == NULL) 
    return program_status_enterprise_menu;
    nk_layout_row_dynamic(ctx, ENTERPRISE_WIDGET_HEIGHT, 1);
    nk_label(ctx, "Welcome to Enterprise", NK_TEXT_CENTERED);
    
    nk_layout_row_template_begin(ctx, ENTERPRISE_WIDGET_HEIGHT);
    nk_layout_row_template_push_static(ctx, 150);
    nk_layout_row_template_push_dynamic(ctx);
    nk_layout_row_template_end(ctx);

    nk_label(ctx, "Name: ", NK_TEXT_LEFT);
    nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, enterprise->name,\
    ENTERPRISE_STRING_LENGTH, nk_filter_default);

    nk_label(ctx, "Balance: ", NK_TEXT_LEFT);
    nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, enterprise->balance,\
    ENTERPRISE_STRING_LENGTH, nk_filter_default);

    nk_layout_row_dynamic(ctx, ENTERPRISE_WIDGET_HEIGHT, 1);
    if (nk_button_label(ctx, "Facilities")) {
        return program_status_facility_table;
    }
    if (nk_button_label(ctx, "Employees")) {
        return program_status_employee_table;
    }
    if (nk_button_label(ctx, "Inventory")) {
        return program_status_item_table;
    }
    if (nk_button_label(ctx, "Customers")) {
        return program_status_customer_table;
    }
    if (nk_button_label(ctx, "Suppliers")) {
        return program_status_supplier_table;
    }
    if (nk_button_label(ctx, "Expenses")) {
        return program_status_expense_table;
    }
    if (nk_button_label(ctx, "Orders")) {
        return program_status_order_table;
    }
    return program_status_enterprise_menu;
}