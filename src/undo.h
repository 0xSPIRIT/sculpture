#ifndef UNDO_H_
#define UNDO_H_

// This undo system stores the state of all grid layers
// in a linked list. Upon saving a state, all previously
// created next pointers from the current state is destroyed.

#include <stdbool.h>

#include "grid.h"

struct Save_State {
    struct Cell *grid_layers[NUM_GRID_LAYERS];
    struct Save_State *prev, *next;
};

extern struct Save_State *current_state, *start_state;

void undo_system_init();
void undo_system_deinit();
bool is_current_grid_same_as(struct Save_State *state);
void save_state_to_next();
struct Save_State *get_state_index(int index);
void free_save_state(struct Save_State *state);
void set_state(struct Save_State *state);
void set_state_to_string_hook(const char *string);
void undo();
void view_save_state_linked_list();

#endif // UNDO_H_
