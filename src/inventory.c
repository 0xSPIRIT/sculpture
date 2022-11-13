void inventory_setup_slots() {
    const int startx = (gs->gw*gs->S)/2 - 0.5*INVENTORY_SLOT_COUNT*100;
    const int starty = GUI_H/2;
    
    for (int i = 0; i < INVENTORY_SLOT_COUNT; i++) {
        gs->inventory.slots[i].x = startx + i * 100 + ITEM_SIZE;
        gs->inventory.slots[i].y = starty;
        gs->inventory.slots[i].w = ITEM_SIZE;
        gs->inventory.slots[i].h = ITEM_SIZE;
        gs->inventory.slots[i].inventory_index = i;
        
        char name[32] = {0};
        sprintf(name, "Slot %d", i+1);
        strcpy(gs->inventory.slots[i].name, name);
    }
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
    
    SDL_Rect r = {
        x, y,
        w, h
            
    };
    SDL_RenderCopy(gs->renderer, gs->textures.items[item->type], NULL, &r);
    
    char number[32] = {0};
    sprintf(number, "%d", item->amount);
    
    SDL_Color color = WHITE;
    
    SDL_Surface *surf = TTF_RenderText_LCD(gs->fonts.font_bold_small, number, color, (SDL_Color){0, 0, 0, 255});
    SDL_Texture *texture = SDL_CreateTextureFromSurface(gs->renderer, surf);
    
    SDL_Rect dst = {
        x + w - surf->w - 1,
        y + h - surf->h - 1,
        surf->w,
        surf->h
    };
    
    SDL_RenderCopy(gs->renderer, texture, NULL, &dst);
    
    SDL_FreeSurface(surf);
    SDL_DestroyTexture(texture);
}

// rx, ry are relative values.
void slot_draw(struct Slot slot, float rx, float ry) {
    SDL_Rect bounds = {
        (int) (rx + slot.x - slot.w/2),
        (int) (ry + slot.y - slot.h/2),
        (int) slot.w,
        (int) slot.h
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
    if (*slot.name) {
        // Finding the allocated surface and textures to use for this.
        
        struct Converter *c = slot.converter;
        
        SDL_Surface **surf;
        SDL_Texture **texture;
        
        if (c) {
            surf = &gs->surfaces.slot_names[INVENTORY_SLOT_COUNT + SLOT_MAX_COUNT * c->type + slot.type];
            texture = &gs->textures.slot_names[INVENTORY_SLOT_COUNT + SLOT_MAX_COUNT * c->type + slot.type];
        } else {
            surf = &gs->surfaces.slot_names[slot.inventory_index];
            texture = &gs->textures.slot_names[slot.inventory_index];
        }
        
        if (!*surf) {
            *surf = TTF_RenderText_LCD(gs->fonts.font_small,
                                       slot.name,
                                       (SDL_Color){0, 0, 0, 255},
                                       (SDL_Color){235, 235, 235, 255});
        }
        Assert(*surf);
        
        if (!*texture) {
            *texture = SDL_CreateTextureFromSurface(gs->renderer, *surf);
        }
        
        Assert(*texture);
        
        SDL_Rect dst = {
            (int) (bounds.x + slot.w/2 - (*surf)->w/2),
            (int) (bounds.y - (*surf)->h - 2),
            (*surf)->w,
            (*surf)->h
        };
        SDL_RenderCopy(gs->renderer, *texture, NULL, &dst);
    }
    
    item_draw(&slot.item, bounds.x, bounds.y, bounds.w, bounds.h);
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

bool is_mouse_in_converter(struct Converter *converter) {
    struct Input *input = &gs->input;
    
    return is_point_in_rect((SDL_Point){
                                input->real_mx,
                                input->real_my
                            },
                            (SDL_Rect){
                                (int)converter->x,
                                (int)converter->y,
                                (int)converter->w,
                                (int)converter->h
                            });
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
    
    // If we're NOT holding down ctrl...
    if (!gs->gui.is_placer_active) {
        can_place_item = can_place_item_in_slot(gs->item_holding.type, slot->type);
        if (!gs->item_holding.type) can_place_item = true;
        
        if (input->mouse_pressed[SDL_BUTTON_LEFT]) {
            // If they're the same type, just add their amounts.
            if (item->type && gs->item_holding.type == item->type) {
                // Add the amount from holding to the item.
                item->amount += gs->item_holding.amount;
                
                gs->item_holding.type = 0;
                gs->item_holding.amount = 0;
            }
            
            // Otherwise if we're either holding an item or have an item in slot,
            // swap them since they're different types.
            else if ((gs->item_holding.type || item->type) && can_place_item) {
                struct Item a = *item;
                
                item->type = gs->item_holding.type;
                item->amount = gs->item_holding.amount;
                
                gs->item_holding.type = a.type;
                gs->item_holding.amount = a.amount;
                
                tooltip_reset(&gs->gui.tooltip);
            }
            
        } else if (input->mouse_pressed[SDL_BUTTON_RIGHT] && item->type && gs->item_holding.type == 0) { // If holding nothing
            // Split the amount into two like minecraft.
            Assert(gs->item_holding.amount == 0);
            
            const int half = item->amount/2;
            
            if (half) {
                item->amount -= half;
                gs->item_holding.type = item->type;
                gs->item_holding.amount += half;
                
                tooltip_reset(&gs->gui.tooltip);
            }
        }
    }
    // If we ARE holding down ctrl...
    else if (gs->gui.is_placer_active) {
        struct Placer *p = get_current_placer();
        struct Converter *converter = NULL;
        
        can_place_item = can_place_item_in_slot(p->contains_type, slot->type);
        
        if (is_mouse_in_converter(gs->material_converter)) {
            converter = gs->material_converter;
        } else if (is_mouse_in_converter(gs->fuel_converter)) {
            converter = gs->fuel_converter;
        }
        
        if (input->mouse_pressed[SDL_BUTTON_RIGHT]) {
            if (p->contains_amount == 0) p->contains_type = 0;
            if (p->contains_type == 0 || p->contains_type == item->type) {
                p->contains_type = item->type;
                p->contains_amount += item->amount;
                
                item->type = 0;
                item->amount = 0;
            }
        } else if (can_place_item && input->mouse & SDL_BUTTON_LEFT) {
            const bool place_instantly = true;
            
            int amt = 0;
            int place_speed = 6;
            
            if (place_instantly) {
                place_speed = p->contains_amount;
            } else {
                // Make sure you handle the case where place_speed > p->contains_amount!
                Assert(0);
            }
            
            amt = place_speed;
            
            if (p->contains_amount && (!item->type || !item->amount)) {
                item->type = p->contains_type;
            }
            
            item->amount += amt;
            p->contains_amount -= amt;
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
}

void inventory_tick() {
    for (int i = 0; i < INVENTORY_SLOT_COUNT; i++) {
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
        slot_draw(gs->inventory.slots[i], 0, 0);
    }
    
    struct Input *input = &gs->input;
    item_draw(&gs->item_holding, input->real_mx - ITEM_SIZE/2, input->real_my - ITEM_SIZE/2, ITEM_SIZE, ITEM_SIZE);
}