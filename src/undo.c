void undo_system_reset() {
    for (int i = 0; i < MAX_UNDO; i++) {
        for (int j = 0; j < NUM_GRID_LAYERS; j++) {
            if (gs->save_states[i].grid_layers[j]) {
                memset(gs->save_states[i].grid_layers[j], 0, gs->gw*gs->gh*sizeof(struct Cell));
            }
        }
    }
}

struct Save_State *current_state() {
    return &gs->save_states[gs->save_state_count-1];
}

void undo_system_init() {
    gs->undo_initialized = true;

    for (int i = 0; i < NUM_GRID_LAYERS; i++) {
        current_state()->grid_layers[i] = arena_alloc(gs->persistent_memory, gs->gw*gs->gh, sizeof(struct Cell));
        memcpy(current_state()->grid_layers[i], gs->grid_layers[i], sizeof(struct Cell)*gs->gw*gs->gh);
    }

    Uint64 size = sizeof(struct Cell) * gs->gw * gs->gh * NUM_GRID_LAYERS * MAX_UNDO;
    printf("Size: %.2f MB\n", (f64)size/(1024.f*1024.f));
}

bool is_current_grid_same_as(struct Save_State *state) {
    return memcmp(gs->grid_layers[0], state->grid_layers[0], gs->gw*gs->gh*sizeof(struct Cell)) == 0;
}
    
void save_state_to_next() {
    if (gs->save_state_count == MAX_UNDO) {
        // Move everything back by one, destroying the first
        // save state and leaving the last slot open.

        gs->save_state_count--;

        for (int i = 0; i < gs->save_state_count; i++) {
            // gs->save_states[i] = gs->save_states[i+1];
            for (int j = 0; j < NUM_GRID_LAYERS; j++) {
                memcpy(gs->save_states[i].grid_layers[j], gs->save_states[i+1].grid_layers[j], sizeof(struct Cell)*gs->gw*gs->gh);
            }
        }

        int i = gs->save_state_count;
        for (int j = 0; j < NUM_GRID_LAYERS; j++) {
            if (gs->save_states[i].grid_layers[j]) {
                memset(gs->save_states[i].grid_layers[j], 0, gs->gw*gs->gh*sizeof(struct Cell));
            }
        }
    }

    struct Save_State *state;
    state = &gs->save_states[gs->save_state_count++];

    // We only need to allocate these once per.
    // After an undo_system_reset() this allocation
    // is still used, the grid layer itself is just
    // zeroed out.
    if (!state->grid_layers[0]) {
        for (int i = 0; i < NUM_GRID_LAYERS; i++) {
            state->grid_layers[i] = arena_alloc(gs->persistent_memory, gs->gw*gs->gh, sizeof(struct Cell));
        }
    }

    for (int i = 0; i < NUM_GRID_LAYERS; i++) {
        memcpy(state->grid_layers[i], gs->grid_layers[i], sizeof(struct Cell)*gs->gw*gs->gh);
    }
}

void set_state(int num) {
    struct Save_State *state = &gs->save_states[num];

    Assert(state->grid_layers[0]);

    for (int i = 0; i < NUM_GRID_LAYERS; i++) {
        memcpy(gs->grid_layers[i], state->grid_layers[i], sizeof(struct Cell)*gs->gw*gs->gh);
    }
}

void set_state_to_string_hook(const char *string) {
    int state_num = atoi(string);
    set_state(state_num);
}

void undo() {
    if (is_current_grid_same_as(current_state())) {
        if (gs->save_state_count == 1) {
            return;
        }

        set_state(gs->save_state_count-2);
        gs->save_state_count--;
    } else { // Just reset to the current state
        set_state(gs->save_state_count-1);
    }

    objects_reevaluate();
}
