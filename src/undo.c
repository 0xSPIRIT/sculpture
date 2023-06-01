static void undo_system_init(void) {
    gs->undo_initialized = true;
    
    gs->save_state_count = 1;
    
    for (int i = 0; i < NUM_GRID_LAYERS; i++) {
        current_state()->grid_layers[i] = PushArray(gs->persistent_memory, gs->gw*gs->gh, sizeof(Cell));
    }
    
    gs->save_state_count = 0;
    
    save_state_to_next();
}

static void undo_system_reset(void) {
    gs->save_state_count = 0;
    for (int i = 0; i < MAX_UNDO; i++) {
        for (int j = 0; j < NUM_GRID_LAYERS; j++) {
            if (gs->save_states[i].grid_layers[j]) {
                memset(gs->save_states[i].grid_layers[j], 0, gs->gw*gs->gh*sizeof(Cell));
            }
        }
        memset(gs->save_states[i].placer_items, 0, sizeof(Item)*TOTAL_SLOT_COUNT);
    }
}

static Save_State *current_state(void) {
    if (gs->save_state_count == 0) return NULL;
    return &gs->save_states[gs->save_state_count-1];
}

static void save_state_to_next(void) {
    if (gs->save_state_count == MAX_UNDO) {
        // Move everything back by one, destroying the first
        // save state and leaving the last slot open.
        
        gs->save_state_count--;
        
        for (int i = 0; i < gs->save_state_count; i++) {
            // gs->save_states[i] = gs->save_states[i+1];
            for (int j = 0; j < NUM_GRID_LAYERS; j++) {
                memcpy(gs->save_states[i].grid_layers[j], gs->save_states[i+1].grid_layers[j], sizeof(Cell)*gs->gw*gs->gh);
            }
            for (int j = 0; j < TOTAL_SLOT_COUNT+1; j++) {
                gs->save_states[i].placer_items[j] = gs->save_states[i+1].placer_items[j];
            }
            
            for (int j = 0; j < SOURCE_CELL_MAX; j++) {
                gs->save_states[i].source_cell[j] = gs->save_states[i+1].source_cell[j];
            }
        }
        
        int i = gs->save_state_count;
        for (int j = 0; j < NUM_GRID_LAYERS; j++) {
            if (gs->save_states[i].grid_layers[j]) {
                memset(gs->save_states[i].grid_layers[j], 0, gs->gw*gs->gh*sizeof(Cell));
            }
            memset(gs->save_states[i].placer_items, 0, sizeof(Item)*(TOTAL_SLOT_COUNT+1));
        }
    }
    
    Save_State *state;
    state = &gs->save_states[gs->save_state_count++];
    
    // We only need to allocate these once per.
    // After an undo_system_reset() this allocation
    // is still used, the grid layer itself is just
    // zeroed out.
    if (!state->grid_layers[0]) {
        for (int i = 0; i < NUM_GRID_LAYERS; i++) {
            state->grid_layers[i] = PushArray(gs->persistent_memory, gs->gw*gs->gh, sizeof(Cell));
        }
    }
    
    for (int i = 0; i < NUM_GRID_LAYERS; i++) {
        memcpy(state->grid_layers[i], gs->grid_layers[i], sizeof(Cell)*gs->gw*gs->gh);
    }
    
    // Inventory Slots
    for (int i = 0; i < INVENTORY_SLOT_COUNT; i++) {
        state->placer_items[i] = gs->inventory.slots[i].item;
    }
    // Material Converter
    for (int i = 0; i < SLOT_MAX_COUNT; i++) {
        state->placer_items[INVENTORY_SLOT_COUNT + i] =
            gs->material_converter->slots[i].item;
    }
    // Fuel Converter
    for (int i = 0; i < SLOT_MAX_COUNT-1; i++) {
        state->placer_items[INVENTORY_SLOT_COUNT + SLOT_MAX_COUNT + i] =
            gs->fuel_converter->slots[i].item;
    }
    state->placer_items[(TOTAL_SLOT_COUNT+1)-1] = gs->item_holding;
    
    // Source Cells
    for (int i = 0; i < SOURCE_CELL_MAX; i++) {
        state->source_cell[i] = gs->levels[gs->level_current].source_cell[i];
    }
    state->source_cell_count = gs->levels[gs->level_current].source_cell_count;
}

static bool is_current_grid_same_as(Save_State *state) {
    for (int i = 0; i < gs->gw*gs->gh; i++) {
        if (state->grid_layers[0][i].type != gs->grid[i].type) return false;
    }
    return true;
}

static bool is_current_slots_same_as(Save_State *state) {
    // Inventory Slots
    for (int i = 0; i < INVENTORY_SLOT_COUNT; i++) {
        if (0 != memcmp(&state->placer_items[i], &gs->inventory.slots[i].item, sizeof(Item))) {
            return false;
        }
    }
    // Material Converter
    for (int i = 0; i < SLOT_MAX_COUNT; i++) {
        if (0 != memcmp(&state->placer_items[INVENTORY_SLOT_COUNT + i], &gs->material_converter->slots[i].item, sizeof(Item))) {
            return false;
        }
    }
    // Fuel Converter
    for (int i = 0; i < SLOT_MAX_COUNT-1; i++) {
        if (0 != memcmp(&state->placer_items[INVENTORY_SLOT_COUNT + SLOT_MAX_COUNT + i], &gs->fuel_converter->slots[i].item, sizeof(Item))) {
            return false;
        }
    }
    // Item Holding
    if (0 != memcmp(&state->placer_items[(TOTAL_SLOT_COUNT+1)-1], &gs->item_holding, sizeof(Item))) {
        return false;
    }
    
    return true;
}

static void set_state(int num) {
    Save_State *state = &gs->save_states[num];
    
    Assert(state->grid_layers[0]);
    
    for (int i = 0; i < NUM_GRID_LAYERS; i++) {
        memcpy(gs->grid_layers[i], state->grid_layers[i], sizeof(Cell)*gs->gw*gs->gh);
    }
    
    // Inventory Slots
    for (int i = 0; i < INVENTORY_SLOT_COUNT; i++) {
        gs->inventory.slots[i].item = state->placer_items[i];
    }
    // Material Converter
    for (int i = 0; i < SLOT_MAX_COUNT; i++) {
        gs->material_converter->slots[i].item =
            state->placer_items[INVENTORY_SLOT_COUNT + i];
    }
    // Fuel Converter
    for (int i = 0; i < SLOT_MAX_COUNT-1; i++) {
        gs->fuel_converter->slots[i].item =
            state->placer_items[INVENTORY_SLOT_COUNT + SLOT_MAX_COUNT + i];
    }
    gs->item_holding = state->placer_items[(TOTAL_SLOT_COUNT+1)-1];
    
    for (int i = 0; i < SOURCE_CELL_MAX; i++) {
        gs->levels[gs->level_current].source_cell[i] = state->source_cell[i];
    }
    gs->levels[gs->level_current].source_cell_count = state->source_cell_count;
}

static void set_state_to_string_hook(const char *string) {
    int state_num = atoi(string);
    set_state(state_num);
}

static void undo(void) {
    if (gs->tutorial.active && strcmp(gs->tutorial.str, TUTORIAL_UNDO_STRING) == 0) {
        tutorial_rect_close(NULL);
    }
    
    if (!gs->gui.popup && is_current_grid_same_as(current_state())) {
        if (gs->save_state_count == 1) {
            return;
        }

        set_state(gs->save_state_count-2);
        gs->save_state_count--;
    } else { 
        if (gs->gui.popup) {
            if (is_current_slots_same_as(current_state())) {
                if (gs->save_state_count == 1) {
                    return;
                }
                
                set_state(gs->save_state_count-2);
                gs->save_state_count--;
            } else {
                set_state(gs->save_state_count-1);
            }
        } else {
            set_state(gs->save_state_count-1);
        }
    }

    objects_reevaluate();
}
