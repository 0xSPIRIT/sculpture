bool is_mouse_in_slot(struct Slot *slot) {
    struct Input *input = &gs->input;
    
    int dx = 0;
    int dy = 0;
    
    if (slot->converter) {
        dx = slot->converter->x;
        dy = slot->converter->y;
    }
    
    return is_point_in_rect((SDL_Point){input->real_mx, input->real_my},
                            (SDL_Rect){
                                (int) (dx + slot->x - slot->w/2),
                                (int) (dy + slot->y - slot->h/2),
                                (int) slot->w,
                                (int) slot->h
                            });
}

bool was_mouse_in_slot(struct Slot *slot) {
    struct Input *input = &gs->input;
    
    int dx = 0;
    int dy = 0;
    
    if (slot->converter) {
        dx = slot->converter->x;
        dy = slot->converter->y;
    }
    
    return is_point_in_rect((SDL_Point){input->real_pmx,input->real_pmy-GUI_H},
                            (SDL_Rect){
                                (int) (dx + slot->x - slot->w/2),
                                (int) (dy + slot->y - slot->h/2),
                                (int) slot->w,
                                (int) slot->h
                            });
}

void inventory_setup_slots() {
    const int startx = (gs->gw*gs->S)/2 - 0.5*INVENTORY_SLOT_COUNT*100;
    const int starty = GUI_H/2;
    
    for (int i = 0; i < INVENTORY_SLOT_COUNT; i++) {
        struct Slot *slot = &gs->inventory.slots[i];
        slot->x = startx + i * 100 + ITEM_SIZE;
        slot->y = starty;
        slot->w = ITEM_SIZE;
        slot->h = ITEM_SIZE;
        slot->inventory_index = i;
        
        char name[32] = {0};
        sprintf(name, "Slot %d", i+1);
        strcpy(gs->inventory.slots[i].name, name);
    }
}

bool add_item_to_inventory_slot(enum Cell_Type type, int amount) {
    for (int i = 0; i < INVENTORY_SLOT_COUNT; i++) {
        struct Slot *slot = &gs->inventory.slots[i];
        if (slot->item.type == 0 ||
            slot->item.type == type)
        {
            slot->item.type = type;
            slot->item.amount += amount;
            return true;
        }
    }
    
    return false;
}

void inventory_init(void) {
    memset(&gs->inventory, 0, sizeof(struct Inventory));
    inventory_setup_slots();
}

void item_draw(struct Item *item, int x, int y, int w, int h) {
    if (!item->type) return;
    if (!item->amount) {
        item->type = 0;
        return;
    }
    
    struct Textures *texs = &gs->textures;
    struct Surfaces *surfs = &gs->surfaces;
    
    SDL_Rect r = {
        x, y,
        w, h
    };
    SDL_RenderCopy(gs->renderer, texs->items[item->type], NULL, &r);
    
    if (gs->item_prev_amounts[item->index] != item->amount) {
        gs->item_prev_amounts[item->index] = item->amount;
        
        char number[32] = {0};
        sprintf(number, "%d", item->amount);
        
        SDL_Color color = WHITE;
        
        if (surfs->item_nums[item->index])
            SDL_FreeSurface(surfs->item_nums[item->index]);
        if (texs->item_nums[item->index])
            SDL_DestroyTexture(texs->item_nums[item->index]);
        
        surfs->item_nums[item->index] = TTF_RenderText_LCD(gs->fonts.font_bold_small,
                                                           number,
                                                           color,
                                                           (SDL_Color){0, 0, 0, 255});
        texs->item_nums[item->index] = SDL_CreateTextureFromSurface(gs->renderer,
                                                                    surfs->item_nums[item->index]);
    }
    
    
    SDL_Rect dst = {
        x + w - surfs->item_nums[item->index]->w - 1,
        y + h - surfs->item_nums[item->index]->h - 1,
        surfs->item_nums[item->index]->w,
        surfs->item_nums[item->index]->h
    };
    SDL_RenderCopy(gs->renderer, texs->item_nums[item->index], NULL, &dst);
}

// rx, ry are relative values.
void slot_draw(struct Slot *slot, f32 rx, f32 ry) {
    SDL_Rect bounds = {
        (int) (rx + slot->x - slot->w/2),
        (int) (ry + slot->y - slot->h/2),
        (int) slot->w,
        (int) slot->h
    };
    
    int col = 200;
    SDL_SetRenderDrawColor(gs->renderer, col, col, col, 255);
    SDL_RenderFillRect(gs->renderer, &bounds);
    
    col = 0;
    SDL_SetRenderDrawColor(gs->renderer, col, col, col, 255);
    
    bounds.x--;
    bounds.y--;
    bounds.w += 2;
    bounds.h += 2;
    
    SDL_RenderDrawRect(gs->renderer, &bounds);
    
    bounds.x++;
    bounds.y++;
    bounds.w -= 2;
    bounds.h -= 2;
    
    // Drawing the slot's name.
    if (*slot->name) {
        // Finding the allocated surface and textures to use for this.
        
        struct Converter *c = slot->converter;
        
        SDL_Surface **surf;
        SDL_Texture **texture;
        
        if (c) {
            surf = &gs->surfaces.slot_names[INVENTORY_SLOT_COUNT + SLOT_MAX_COUNT * c->type + slot->type];
            texture = &gs->textures.slot_names[INVENTORY_SLOT_COUNT + SLOT_MAX_COUNT * c->type + slot->type];
        } else {
            surf = &gs->surfaces.slot_names[slot->inventory_index];
            texture = &gs->textures.slot_names[slot->inventory_index];
        }
        
        if (!*surf) {
            *surf = TTF_RenderText_LCD(gs->fonts.font_small,
                                       slot->name,
                                       (SDL_Color){0, 0, 0, 255},
                                       (SDL_Color){235, 235, 235, 255});
        }
        Assert(*surf);
        
        if (!*texture) {
            *texture = SDL_CreateTextureFromSurface(gs->renderer, *surf);
        }
        
        Assert(*texture);
        
        SDL_Rect dst = {
            (int) (bounds.x + slot->w/2 - (*surf)->w/2),
            (int) (bounds.y - (*surf)->h - 2),
            (*surf)->w,
            (*surf)->h
        };
        SDL_RenderCopy(gs->renderer, *texture, NULL, &dst);
    }
    
    item_draw(&slot->item, bounds.x, bounds.y, bounds.w, bounds.h);
}

//////////////////////////////// Inventory Ticking


bool is_cell_fuel(int type) {
    switch (type) {
        case CELL_REFINED_COAL: case CELL_UNREFINED_COAL: case CELL_LAVA:
        return true;
    }
    return false;
}

bool can_place_item_in_slot(int type, enum Slot_Type slot) {
    bool can_put_fuel = false;
    
    if (slot == SLOT_FUEL) {
        can_put_fuel = is_cell_fuel(type);
    }
    
    return
        slot == SLOT_INPUT1 ||
        slot == SLOT_INPUT2 ||
        can_put_fuel;
}

// Slot may be NULL if the item doesn't belong to any slot.
// This function mostly just handles interactions with items and the mouse.
void item_tick(struct Item *item, struct Slot *slot, int x, int y, int w, int h) {
    struct Input *input = &gs->input;
    
    if (item == &gs->item_holding) {
        if (input->real_my < gs->gui.popup_y && 
            input->real_my > GUI_H && 
            input->mouse_pressed[SDL_BUTTON_LEFT]) 
        {
            // We pressed outside of the converter area.
            // We will now kill the holding item.
            gs->item_holding.type = 0;
            gs->item_holding.amount = 0;
        }
        return;
    }
    
    if (!is_point_in_rect((SDL_Point){input->real_mx, input->real_my}, (SDL_Rect){x, y, w, h})) return;
    
    // From this point onwards, we know the mouse is in this item,
    // and this item is not currently being held.
    
    bool can_place_item = false;
    
    can_place_item = can_place_item_in_slot(gs->item_holding.type, slot->type);
    if (!gs->item_holding.type) can_place_item = true;
    
    if (input->mouse_pressed[SDL_BUTTON_LEFT]) {
        // If they're the same type, just add their amounts.
        if (item->type && gs->item_holding.type == item->type) {
            save_state_to_next();
            
            // Add the amount from holding to the item.
            item->amount += gs->item_holding.amount;
            
            gs->item_holding.type = 0;
            gs->item_holding.amount = 0;
        }
        
        // Otherwise if we're either holding an item or have an item in slot,
        // swap them since they're different types.
        else if ((gs->item_holding.type || item->type) && can_place_item) {
            struct Item a = *item;
            
            save_state_to_next();
            
            item->type = gs->item_holding.type;
            item->amount = gs->item_holding.amount;
            
            gs->item_holding.type = a.type;
            gs->item_holding.amount = a.amount;
            
            tooltip_reset(&gs->gui.tooltip);
        }
        
    } else if (input->mouse_pressed[SDL_BUTTON_RIGHT]) {
        if (item->type && gs->item_holding.type == 0) {
            // Split the amount into two like minecraft.
            Assert(gs->item_holding.amount == 0);
            
            const int half = item->amount/2;
            
            if (half) {
                save_state_to_next();
                
                item->amount -= half;
                gs->item_holding.type = item->type;
                gs->item_holding.amount += half;
                
                tooltip_reset(&gs->gui.tooltip);
            }
        } else if (gs->item_holding.type && (item->type == 0 || item->type == gs->item_holding.type)) {
            // Place only one down at a time.
            
            save_state_to_next();
            
            gs->item_holding.amount--;
            item->type = gs->item_holding.type;
            item->amount++;
            
            if (gs->item_holding.amount <= 0) gs->item_holding.type = 0;
        }
    }
}

void slot_tick(struct Slot *slot) {
    int rx = 0, ry = 0;
    
    if (slot->converter) {
        rx = slot->converter->x;
        ry = slot->converter->y;
    }
    
    item_tick(&slot->item,
              slot,
              (int) (rx + slot->x - slot->w/2),
              (int) (ry + slot->y - slot->h/2),
              (int) slot->w,
              (int) slot->h);
    
    if (slot->item.type && slot->item.amount == 0) {
        slot->item.type = 0;
    }
}

// Look in gui.c for the ticking of converter slots.
void inventory_tick() {
    if (!gs->gui.popup) return;
    
    if (gs->level_current == 5-1 && !gs->did_fuel_converter_tutorial) {
        struct Tutorial_Rect *next = tutorial_rect(TUTORIAL_TEXT_FILE_STRING,
                                                   32,
                                                   GUI_H+128,
                                                   NULL);
        gs->tutorial = *tutorial_rect(TUTORIAL_FUEL_CONVERTER_STRING,
                                      32,
                                      GUI_H+128,
                                      next);
        gs->did_fuel_converter_tutorial = true;
    } 
    if (gs->level_current == 3-1 && !gs->did_inventory_tutorial) {
        gs->tutorial = *tutorial_rect(TUTORIAL_INVENTORY_STRING,
                                      32,
                                      GUI_H+128,
                                      NULL);
        gs->did_inventory_tutorial = true;
    }
    
    if (gs->item_holding.type == 0 && gs->item_holding.amount) {
        gs->item_holding.amount = 0;
    }
    
    for (int i = 0; i < INVENTORY_SLOT_COUNT; i++) {
        struct Slot *slot = &gs->inventory.slots[i];
        if (slot->item.type && is_mouse_in_slot(slot)) {
            tooltip_set_position_to_cursor(&gs->gui.tooltip, TOOLTIP_TYPE_ITEM);
            
            char type[64] = {0};
            char type_name[64] = {0};
            char amount[64] = {0};
            
            get_name_from_type(slot->item.type, type_name);
            sprintf(type, "Type: %s", type_name);
            sprintf(amount, "Amount: %d", slot->item.amount);
            
            strcpy(gs->gui.tooltip.str[0], type);
            strcpy(gs->gui.tooltip.str[1], amount);
            
            gs->gui.tooltip.set_this_frame = true;
        } else if (!gs->gui.tooltip.set_this_frame && gs->gui.tooltip.type == TOOLTIP_TYPE_ITEM) {
            tooltip_reset(&gs->gui.tooltip);
        }
        
        slot_tick(&gs->inventory.slots[i]);
    }
    
    
    struct Input *input = &gs->input;
    item_tick(&gs->item_holding, NULL, input->real_mx, input->real_my, ITEM_SIZE, ITEM_SIZE);
}

// calls from gui_draw()
void inventory_draw(void) {
    if (!gs->gui.popup) return;
    
    inventory_setup_slots();
    
    const SDL_Rect rect = {
        0, 0,
        gs->S*gs->gw, GUI_H
    };
    SDL_SetRenderDrawColor(gs->renderer, 235, 235, 235, 255);
    SDL_RenderFillRect(gs->renderer, &rect);
    
    for (int i = 0; i < INVENTORY_SLOT_COUNT; i++) {
        slot_draw(&gs->inventory.slots[i], 0, 0);
    }
    
    struct Input *input = &gs->input;
    item_draw(&gs->item_holding, input->real_mx - ITEM_SIZE/2, input->real_my - ITEM_SIZE/2, ITEM_SIZE, ITEM_SIZE);
}