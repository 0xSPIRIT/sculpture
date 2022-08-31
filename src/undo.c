#include "undo.h"

#include <stdlib.h>
#include <string.h>

struct Save_State *start_save_state, *current_save_state;

void undo_system_init() {
    start_save_state = calloc(1, sizeof(struct Save_State));
    current_save_state = start_save_state;
    for (int i = 0; i < NUM_GRID_LAYERS; i++) {
        current_save_state->grid_layers[i] = calloc(gw*gh, sizeof(struct Cell));
        memcpy(current_save_state->grid_layers[i], grid_layers[i], gw*gh*sizeof(struct Cell));
    }
}

void undo_system_deinit() {
    struct Save_State *next = start_save_state;
    for (struct Save_State *curr = start_save_state; curr; curr = next) {
        next = curr->next;
        deinit_state(curr);
    }
}

void save_state() {
    struct Save_State *old = current_save_state;
    
    // If we have next states, we need to remove all of them until the end.
    if (current_save_state->next) {
        struct Save_State *next = current_save_state->next;

        while (next) {
            struct Save_State *nn = next->next;
            deinit_state(next);
            next = nn;
        }
    }

    current_save_state->next = malloc(sizeof(struct Save_State));
    current_save_state = current_save_state->next;
    current_save_state->prev = old;
    current_save_state->next = NULL;

    for (int i = 0; i < NUM_GRID_LAYERS; i++) {
        current_save_state->grid_layers[i] = calloc(gw*gh, sizeof(struct Cell));
        memcpy(current_save_state->grid_layers[i], grid_layers[i], gw*gh*sizeof(struct Cell));
    }

    printf("Saved State.\n"); fflush(stdout);
}

void deinit_state(struct Save_State *state) {
    for (int i = 0; i < NUM_GRID_LAYERS; i++) {
        free(state->grid_layers[i]);
    }
}

void set_state(struct Save_State *state) {
    for (int i = 0; i < NUM_GRID_LAYERS; i++) {
        memcpy(grid_layers[i], state->grid_layers[i], gw*gh*sizeof(struct Cell));
    }
}

void undo() {
    printf("Undo.\n"); fflush(stdout);

    if (current_save_state->next) {
        struct Save_State *next = current_save_state->next;
        while (next) {
            struct Save_State *nn = next->next;
            deinit_state(next);
            next = nn;
        }
    }

    set_state(current_save_state);
    if (current_save_state->prev) {
        current_save_state = current_save_state->prev;
    }
}

void redo() {
    printf("Redo.\n"); fflush(stdout);

    if (current_save_state->next) {
        set_state(current_save_state->next);
        current_save_state = current_save_state->next;
    }
}

void view_save_state_linked_list() {
    printf("\n\nSTART.\n"); fflush(stdout);
    for (struct Save_State *state = start_save_state; state; state = state->next) {
        printf("Prev: %p\n", (void*) state->prev);
        printf("Me:   %p", (void*) state);
        if (state == current_save_state) {
            printf(" [current]");
        }
        printf("\n");
        printf("Next: %p\n", (void*) state->next);
        if (state->next) {
            printf("    |\n");
            printf("    |\n");
            printf("    v\n");
        } else {
            printf("\nEND.\n");
        }
    }
    fflush(stdout);
}
