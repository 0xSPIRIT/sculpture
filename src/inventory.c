bool is_mouse_in_slot(Slot *slot) {
    Input *input = &gs->input;
    
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

bool was_mouse_in_slot(Slot *slot) {
    Input *input = &gs->input;
    
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
    const int startx = (gs->gw*gs->S)/2 - 0.5*INVENTORY_SLOT_COUNT*Scale(100);
    const int starty = GUI_H/2;
    
    for (int i = 0; i < INVENTORY_SLOT_COUNT; i++) {
        Slot *slot = &gs->inventory.slots[i];
        slot->x = startx + i * Scale(100) + ITEM_SIZE;
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
        Slot *slot = &gs->inventory.slots[i];
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
    memset(&gs->inventory, 0, sizeof(Inventory));
    inventory_setup_slots();
}

void item_draw(Item *item, int x, int y, int w, int h) {
    if (!item->type) return;
    if (!item->amount) {
        item->type = 0;
        return;
    }
    
    Textures *texs = &gs->textures;
    Surfaces *surfs = &gs->surfaces;
    
    SDL_Rect r = {
        x, y,
        w, h
    };
    
    if (y >= GUI_H && y <= gs->gui.popup_y) {
        SDL_SetTextureColorMod(texs->items[item->type], 255, 0, 0);
    } else {
        SDL_SetTextureColorMod(texs->items[item->type], 255, 255, 255);
    }
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
                                                           BLACK);
        texs->item_nums[item->index] = SDL_CreateTextureFromSurface(gs->renderer,
                                                                    surfs->item_nums[item->index]);
    }
    
    
    SDL_Rect dst = {
        x + w - surfs->item_nums[item->index]->w - 1,
        y + h - surfs->item_nums[item->index]->h - 1,
        surfs->item_nums[item->index]->w,
        surfs->item_nums[item->index]->h
    };
    
    if (y >= GUI_H && y <= gs->gui.popup_y) {
        SDL_SetTextureColorMod(texs->item_nums[item->index], 255, 0, 0);
    } else {
        SDL_SetTextureColorMod(texs->item_nums[item->index], 255, 255, 255);
    }
    SDL_RenderCopy(gs->renderer, texs->item_nums[item->index], NULL, &dst);
}

// rx, ry are relative values.
void slot_draw(Slot *slot, f32 rx, f32 ry) {
    SDL_Rect bounds = {
        (int) (rx + slot->x - slot->w/2),
        (int) (ry + slot->y - slot->h/2),
        (int) slot->w,
        (int) slot->h
    };
    
    SDL_SetRenderDrawColor(gs->renderer,
                           Red(SLOT_COLOR),
                           Green(SLOT_COLOR),
                           Blue(SLOT_COLOR),
                           255);
    SDL_RenderFillRect(gs->renderer, &bounds);
    
    if (slot->inventory_index != -1 && slot->inventory_index == gs->current_placer) {
        SDL_SetRenderDrawColor(gs->renderer, 
                               255,
                               255,
                               0,
                               255);
    } else {
        SDL_SetRenderDrawColor(gs->renderer, 
                               Red(SLOT_OUTLINE_COLOR),
                               Green(SLOT_OUTLINE_COLOR),
                               Blue(SLOT_OUTLINE_COLOR),
                               255);
    }
    
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
        
        Converter *c = slot->converter;
        
        SDL_Surface **surf;
        SDL_Texture **texture;
        
        if (c) {
            surf = &gs->surfaces.slot_names[INVENTORY_SLOT_COUNT + SLOT_MAX_COUNT * c->type + slot->type];
            texture = &gs->textures.slot_names[INVENTORY_SLOT_COUNT + SLOT_MAX_COUNT * c->type + slot->type];
        } else {
            surf = &gs->surfaces.slot_names[slot->inventory_index];
            texture = &gs->textures.slot_names[slot->inventory_index];
        }
        
#ifndef MODIFYING_COLORS
        if (!*surf) {
#else
        if (*surf) SDL_FreeSurface(*surf);
#endif
            *surf = TTF_RenderText_Blended(gs->fonts.font_small,
                                       slot->name,
                                       (SDL_Color){
                                           Red(SLOT_TEXT_COLOR), 
                                           Green(SLOT_TEXT_COLOR), 
                                           Blue(SLOT_TEXT_COLOR), 
                                           255
                                           });
#if 0
                                       (SDL_Color){
                                           Red(INVENTORY_COLOR),
                                           Green(INVENTORY_COLOR),
                                           Blue(INVENTORY_COLOR),
                                           0});
#endif
#ifndef MODIFYING_COLORS
        }
#else
        Assert(*surf);
#endif
        
#ifndef MODIFYING_COLORS
        if (!*texture) {
#else
        if (*texture) SDL_DestroyTexture(*texture);
#endif
            *texture = SDL_CreateTextureFromSurface(gs->renderer, *surf);
#ifndef MODIFYING_COLORS
        }
#endif
        
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
void item_tick(Item *item, Slot *slot, int x, int y, int w, int h) {
    Input *input = &gs->input;
    
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
        
        // Firstly, if you're shift-clicking, and you're not holding anything,
        // put it into the correct slot.
        if (can_place_item && item->type && input->keys[SDL_SCANCODE_LSHIFT]) {
            if (slot->inventory_index != -1) { // We are on the inventory
                if (is_cell_fuel(item->type)) {
                    // Swap into fuel if the fuel cell is empty or the same type as our item.
                    Slot *converter_fuel_slot = &gs->material_converter->slots[SLOT_FUEL];
                    if (converter_fuel_slot->item.type == 0 || converter_fuel_slot->item.type == item->type) {
                        converter_fuel_slot->item.type = item->type;
                        converter_fuel_slot->item.amount += item->amount;
                        item->type = 0;
                        item->amount = 0;
                    }
                }
            } else {
                // We are not in the inventory
                // Swap into the inventory index if there's a free spot
                add_item_to_inventory_slot(item->type, item->amount);
                item->type = 0;
                item->amount = 0;
            }
        }
        
        // Otherwise, if they're the same type, just add their amounts.
        else if (item->type && gs->item_holding.type == item->type) {
            save_state_to_next();
            
            // Add the amount from holding to the item.
            item->amount += gs->item_holding.amount;
            
            gs->item_holding.type = 0;
            gs->item_holding.amount = 0;
        }
        
        // Otherwise if we're either holding an item or have an item in slot,
        // swap them since they're different types.
        else if ((gs->item_holding.type || item->type) && can_place_item) {
            Item a = *item;
            
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

void slot_tick(Slot *slot) {
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
    
    if (gs->level_current == 6-1 && !gs->did_fuel_converter_tutorial) {
        Tutorial_Rect *next = tutorial_rect(TUTORIAL_TEXT_FILE_STRING,
                                                   NormX(32),
                                                   NormY((768.8/8.0)+128),
                                                   NULL);
        gs->tutorial = *tutorial_rect(TUTORIAL_FUEL_CONVERTER_STRING,
                                      NormX(32),
                                      NormY((768.8/8.0)+128),
                                      next);
        gs->did_fuel_converter_tutorial = true;
    } 
    if (gs->level_current == 4-1 && !gs->did_inventory_tutorial) {
        gs->tutorial = *tutorial_rect(TUTORIAL_INVENTORY_STRING,
                                      NormX(32),
                                      NormY((768.8/8.0)+128),
                                      NULL);
        gs->did_inventory_tutorial = true;
    }
    
    if (gs->item_holding.type == 0 && gs->item_holding.amount) {
        gs->item_holding.amount = 0;
    }
    
    for (int i = 0; i < INVENTORY_SLOT_COUNT; i++) {
        Slot *slot = &gs->inventory.slots[i];
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
    
    
    Input *input = &gs->input;
    item_tick(&gs->item_holding, NULL, input->real_mx, input->real_my, ITEM_SIZE, ITEM_SIZE);
}

// calls from gui_draw()
void inventory_draw(void) {
    if (gs->gui.popup_inventory_y <= 0) return;
    
    GUI *gui = &gs->gui;
    
    inventory_setup_slots();
    
    const f32 y = -GUI_H + gui->popup_inventory_y;
    
    const SDL_Rect rect = {
        0, y,
        gs->S*gs->gw, GUI_H
    };
    SDL_SetRenderDrawColor(gs->renderer, 
                           Red(INVENTORY_COLOR),
                           Green(INVENTORY_COLOR),
                           Blue(INVENTORY_COLOR),
                           255);
    
    SDL_RenderFillRect(gs->renderer, &rect);
    
    SDL_SetRenderDrawColor(gs->renderer, 
                           Red(CONVERTER_LINE_COLOR),
                           Green(CONVERTER_LINE_COLOR),
                           Blue(CONVERTER_LINE_COLOR),
                           255);
    SDL_RenderDrawLine(gs->renderer,
                       0,
                       y+GUI_H,
                       gs->window_width,
                       y+GUI_H);
    
    for (int i = 0; i < INVENTORY_SLOT_COUNT; i++) {
        slot_draw(&gs->inventory.slots[i], 0, y);
    }
    
    Input *input = &gs->input;
    item_draw(&gs->item_holding,
              input->real_mx - ITEM_SIZE/2,
              y + input->real_my - ITEM_SIZE/2,
              ITEM_SIZE,
              ITEM_SIZE);
}