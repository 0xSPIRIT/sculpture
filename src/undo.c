#include "undo.h"

#include <stdlib.h>

struct Save_State *current_state, *start_state;

void undo_system_init() {
    start_state = calloc(1, sizeof(struct Save_State));
    current_state = start_state;

    for (int i = 0; i < NUM_GRID_LAYERS; i++) {
        current_state->grid_layers[i] = calloc(gw*gh, sizeof(struct Cell));
        memcpy(current_state->grid_layers[i], grid_layers[i], sizeof(struct Cell)*gw*gh);
    }
}

void undo_system_deinit() {
    struct Save_State *next = NULL;
    for (struct Save_State *state = start_state; state; state = next) {
        next = state->next;
        free_save_state(state);
    }
}

bool is_current_grid_same_as(struct Save_State *state) {
    // grid == grid_layers[0]
    return memcmp(grid_layers[0], state->grid_layers[0], gw*gh*sizeof(struct Cell)) == 0;
}
    
void save_state_to_next() {
    struct Save_State *state;
    state = calloc(1, sizeof(struct Save_State));

    if (!state->grid_layers[0]) {
        for (int i = 0; i < NUM_GRID_LAYERS; i++) {
            state->grid_layers[i] = calloc(gw*gh, sizeof(struct Cell));
        }
    }

    for (int i = 0; i < NUM_GRID_LAYERS; i++) {
        memcpy(state->grid_layers[i], grid_layers[i], sizeof(struct Cell)*gw*gh);
    }

    current_state->next = state;
    state->prev = current_state;
    current_state = state;
}

struct Save_State *get_state_index(int index) {
    struct Save_State *s = start_state;
    for (int i = 0; i < index; i++) {
        if (s->next) {
            s = s->next;
        } else {
            break;
        }
    }
    return s;
}

void free_save_state(struct Save_State *state) {
    for (int i = 0; i < NUM_GRID_LAYERS; i++) {
        free(state->grid_layers[i]);
    }
    free(state);
}

void set_state(struct Save_State *state) {
    for (int i = 0; i < NUM_GRID_LAYERS; i++) {
        memcpy(grid_layers[i], state->grid_layers[i], sizeof(struct Cell)*gw*gh);
    }
}

void set_state_to_string_hook(const char *string) {
    int state_num = atoi(string);
    set_state(get_state_index(state_num));
}

void undo() {
    if (is_current_grid_same_as(current_state)) {
        if (!current_state->prev) {
            return;
        }
        set_state(current_state->prev);
        current_state = current_state->prev;
        free_save_state(current_state->next);
        current_state->next = NULL;
    } else { // Just reset to the current state
        set_state(current_state);
    }
}

void view_save_state_linked_list() {
    printf("\n\nSTART.\n"); fflush(stdout);
    for (struct Save_State *state = start_state; state; state = state->next) {
        printf("Me:   %p", (void*) state);
        if (state == current_state) {
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
