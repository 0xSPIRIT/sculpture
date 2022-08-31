#ifndef UNDO_H_
#define UNDO_H_

// This undo system stores the state of all grid layers
// in a linked list. Upon saving a state, all previously
// created next pointers from the current state is destroyed.

#include "grid.h"

struct Save_State {
    struct Cell *grid_layers[NUM_GRID_LAYERS];
    struct Save_State *prev, *next;
};

extern struct Save_State *start_save_state, *current_save_state;

void undo_system_init();
void undo_system_deinit();
void save_state();
void set_state(struct Save_State *state);
void deinit_state(struct Save_State *state);
void undo();
void redo();
void view_save_state_linked_list();

#endif // UNDO_H_
