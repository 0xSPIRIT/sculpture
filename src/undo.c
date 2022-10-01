#include "undo.h"

#include <stdio.h>
#include <stdlib.h>

#include "game.h"

void undo_system_init() {
    gs->start_state = arena_alloc(gs->persistent_memory, 1, sizeof(struct Save_State));
    gs->current_state = gs->start_state;

    for (int i = 0; i < NUM_GRID_LAYERS; i++) {
        gs->current_state->grid_layers[i] = arena_alloc(gs->persistent_memory, gs->gw*gs->gh, sizeof(struct Cell));
        memcpy(gs->current_state->grid_layers[i], gs->grid_layers[i], sizeof(struct Cell)*gs->gw*gs->gh);
    }
}

bool is_current_grid_same_as(struct Save_State *state) {
    return memcmp(gs->grid_layers[0], state->grid_layers[0], gs->gw*gs->gh*sizeof(struct Cell)) == 0;
}
    
void save_state_to_next() {
    if (gs->state_count >= MAX_UNDO) {
        return;
    }

    struct Save_State *state;
    state = arena_alloc(gs->persistent_memory, 1, sizeof(struct Save_State));

    if (!state->grid_layers[0]) {
        for (int i = 0; i < NUM_GRID_LAYERS; i++) {
            state->grid_layers[i] = arena_alloc(gs->persistent_memory, gs->gw*gs->gh, sizeof(struct Cell));
        }
    }

    for (int i = 0; i < NUM_GRID_LAYERS; i++) {
        memcpy(state->grid_layers[i], gs->grid_layers[i], sizeof(struct Cell)*gs->gw*gs->gh);
    }

    gs->current_state->next = state;
    state->prev = gs->current_state;
    gs->current_state = state;

    gs->state_count++;
}

struct Save_State *get_state_index(int index) {
    struct Save_State *s = gs->start_state;
    for (int i = 0; i < index; i++) {
        if (s->next) {
            s = s->next;
        } else {
            break;
        }
    }
    return s;
}

void set_state(struct Save_State *state) {
    for (int i = 0; i < NUM_GRID_LAYERS; i++) {
        memcpy(gs->grid_layers[i], state->grid_layers[i], sizeof(struct Cell)*gs->gw*gs->gh);
    }
}

void set_state_to_string_hook(const char *string) {
    int state_num = atoi(string);
    set_state(get_state_index(state_num));
}

void undo() {
    if (is_current_grid_same_as(gs->current_state)) {
        if (!gs->current_state->prev) {
            return;
        }
        set_state(gs->current_state->prev);
        gs->current_state = gs->current_state->prev;
        gs->current_state->next = NULL;
        gs->state_count--;
    } else { // Just reset to the current state
        set_state(gs->current_state);
    }
    objects_reevaluate();
}

void view_save_state_linked_list() {
    printf("\n\nSTART.\n");
    for (struct Save_State *state = gs->start_state; state; state = state->next) {
        printf("Me:   %p", (void*) state);
        if (state == gs->current_state) {
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
}
