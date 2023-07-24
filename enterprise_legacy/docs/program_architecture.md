# Enterprise Documentation

## How the program works:
- It holds a state that stores rendering information, status information and the
enterprise database itself.

- It initialises all the required inforamtion, and enters into a program loop
that handles user input and draws the GUI interface to the screen.

## Rendering:
- This program uses OpenGL, SDL and the Nuklear GUI toolkit to render graphics.

## The Enterprise Struct:
- This struct contains all the data related to the enterprise.
- It contains pointers to linked lists that store related objects and metadata 
for the mentioned linked lists.

## How facilities work.
- Facilities are stored in a struct that contains a pointer to the head of a
linked list containing all the facilities.

- The struct that stores the linked list is directly added to the enterprise struct and stores metadata about the list.

- All facilities are assigned a unique ID, no two facilities can have the
same ID, the facility_list structure keeps track of that.

- The metadata includes the last unique ID, as well as the currently selected
facility.

- This is so the facility editor knows what is the correct facility to edit.

- ### Facility Data structures:
    - `facility_node`: An individual facility.

    - `facility_list`: A structure holding important metadata about the facility linked list.
