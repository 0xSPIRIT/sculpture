static bool is_mouse_in_slot(Slot *slot) {
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

static bool was_mouse_in_slot(Slot *slot) {
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

static void set_slot(int index, Cell_Type type, int amount) {
    Slot *slots = gs->inventory.slots;
    slots[index].item.type = type;
    slots[index].item.amount = amount;
}

static void auto_set_inventory_slots(void) {
    int level = gs->level_current+1;

    switch (level) {
        case 6: {
            set_slot(0, CELL_STONE, 1000);
            set_slot(1, CELL_SAND,  1000);
        } break;
        case 7: {
            set_slot(0, CELL_GLASS,          350);
            set_slot(1, CELL_SANDSTONE,      350);
            set_slot(2, CELL_UNREFINED_COAL, 600);
        } break;
        case 8: {
            set_slot(0, CELL_STONE, 1500);
            set_slot(1, CELL_SAND,  2000);
        } break;
        case 9: {
            set_slot(0, CELL_MARBLE, 1300);
            set_slot(1, CELL_SAND,   1300);
            set_slot(2, CELL_DIRT,   1300);
        } break;
    }
}

static void inventory_setup_slots(void) {
    const int startx = (gs->gh*gs->S)/2 - 0.5*INVENTORY_SLOT_COUNT*Scale(100);
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

static bool can_add_item_to_inventory(enum Cell_Type type) {
    for (int i = 0; i < INVENTORY_SLOT_COUNT; i++) {
        Slot *slot = &gs->inventory.slots[i];
        if (slot->item.type == 0 ||
            slot->item.type == type)
        {
            return true;
        }
    }

    return false;
}

// TODO: If there's an existing spot for `type`, add it to that slot
//       instead of the first available one.
static bool add_item_to_inventory_slot(enum Cell_Type type, int amount) {
    int index = -1;
    for (int i = 0; i < INVENTORY_SLOT_COUNT; i++) {
        Slot *slot = &gs->inventory.slots[i];
        if (slot->item.type == type) {
            index = i;
            break;
        }
    }

    if (index == -1) {
        for (int i = 0; i < INVENTORY_SLOT_COUNT; i++) {
            Slot *slot = &gs->inventory.slots[i];
            if (slot->item.type == 0) {
                index = i;
                break;
            }
        }
    }

    if (index == -1) return false;

    Slot *slot = &gs->inventory.slots[index];
    slot->item.type = type;
    slot->item.amount += amount;
    return true;
}

static void inventory_init(void) {
    memset(&gs->inventory, 0, sizeof(Inventory));
    inventory_setup_slots();
    auto_set_inventory_slots();
}

static void item_draw(int target, Item item, int x, int y, int w, int h) {
    if (!item.type) return;
    if (!item.amount) {
        item.type = 0;
        return;
    }

    SDL_Rect r = {
        x, y,
        w, h
    };

    if (y >= GUI_H && y <= gs->gui.popup_y) {
        RenderTextureColorMod(&GetTexture(TEXTURE_ITEMS+item.type), 255, 0, 0);
    } else {
        RenderTextureColorMod(&GetTexture(TEXTURE_ITEMS+item.type), 255, 255, 255);
    }

    RenderTexture(target,
                  &GetTexture(TEXTURE_ITEMS+item.type),
                  null,
                  &r);

    Render_Text_Data text_data = {0};
    sprintf(text_data.identifier, "item %d", item.index);
    text_data.font = gs->fonts.font_bold_small;
    sprintf(text_data.str, "%d", item.amount);
    if (y >= GUI_H && y <= gs->gui.popup_y) {
        text_data.foreground = (SDL_Color){255, 0, 0, 255};
    } else {
        text_data.foreground = WHITE;
    }
    text_data.background = BLACK;
    text_data.x = x+w;
    text_data.y = y+h;
    text_data.alignment = ALIGNMENT_BOTTOM_RIGHT;
    text_data.render_type = TEXT_RENDER_LCD;

    RenderText(target, &text_data);
}

// rx, ry are relative values.
static void slot_draw(int target, Slot *slot, f32 rx, f32 ry) {
    SDL_Rect bounds = {
        (int) (rx + slot->x - slot->w/2),
        (int) (ry + slot->y - slot->h/2),
        (int) slot->w,
        (int) slot->h
    };

    RenderColor(Red(SLOT_COLOR),
                Green(SLOT_COLOR),
                Blue(SLOT_COLOR),
                255);
    RenderFillRect(target, bounds);
    
    bool thick_outline = false;

    if (slot->inventory_index != -1 && slot->inventory_index == gs->current_placer) {
        RenderColorStruct(ColorFromInt(SLOT_OUTLINE_SELECTED_COLOR));
        thick_outline = true;
    } else {
        RenderColorStruct(ColorFromInt(SLOT_OUTLINE_COLOR));
    }
    
    SDL_Rect stored_bounds = bounds;
    
    bounds.x--;
    bounds.y--;
    bounds.w += 2;
    bounds.h += 2;
    
    RenderDrawRect(target, bounds);
    if (thick_outline) {
        bounds.x--;
        bounds.y--;
        bounds.w += 2;
        bounds.h += 2;
        RenderDrawRect(target, bounds);
    }
    
    bounds = stored_bounds;
    
    // Drawing the slot's name and F key
    if (*slot->name) {
        { // Slot name
            Render_Text_Data text_data = {0};

            text_data.font = gs->fonts.font_small;
            sprintf(text_data.identifier, "Slot %p thing", slot);
            strcpy(text_data.str, slot->name);
            text_data.foreground = (SDL_Color){
                Red(SLOT_TEXT_COLOR),
                Green(SLOT_TEXT_COLOR),
                Blue(SLOT_TEXT_COLOR),
                255
            };
            text_data.x = (int) (bounds.x + slot->w/2);
            text_data.y = (int) (bounds.y - Scale(10));
            text_data.alignment = ALIGNMENT_CENTER;
            text_data.render_type = TEXT_RENDER_BLENDED;

            RenderText(target, &text_data);
        }

        if (slot->inventory_index != -1) { // F key
            Render_Text_Data text_data = {0};

            char msg[10];
            sprintf(msg, "F%d", slot->inventory_index+1);

            text_data.font = gs->fonts.font_small;
            sprintf(text_data.identifier, "sdf%p", slot);
            strcpy(text_data.str, msg);
            text_data.foreground = (SDL_Color){
                Red(SLOT_TEXT_COLOR),
                Green(SLOT_TEXT_COLOR),
                Blue(SLOT_TEXT_COLOR),
                255
            };
            text_data.x = (int) (bounds.x + slot->w/2);
            text_data.y = (int) (bounds.y + bounds.h + Scale(13));
            text_data.alignment = ALIGNMENT_CENTER;
            text_data.render_type = TEXT_RENDER_BLENDED;

            RenderText(target, &text_data);
        }
    }

    item_draw(target, slot->item, bounds.x, bounds.y, bounds.w, bounds.h);
}

//////////////////////////////// Inventory Ticking


static bool is_cell_fuel(int type) {
    switch (type) {
        case CELL_REFINED_COAL: case CELL_UNREFINED_COAL: case CELL_LAVA:
        return true;
    }
    return false;
}

static bool can_place_item_in_slot(int type, enum Slot_Type slot) {
    bool can_put_fuel = false;

    if (slot == SLOT_FUEL) {
        can_put_fuel = is_cell_fuel(type);
    }

    return
        slot == SLOT_INPUT1 ||
        slot == SLOT_INPUT2 ||
        can_put_fuel;
}

// Slot may be null if the item doesn't belong to any slot.
// This function mostly just handles interactions with items and the mouse.
static void item_tick(Item *item, Slot *slot, int x, int y, int w, int h) {
    Input *input = &gs->input;

    if (gs->recipes.active) return;

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
    bool can_take_item = true;

    can_place_item = can_place_item_in_slot(gs->item_holding.type, slot->type);
    if (!gs->item_holding.type)
        can_place_item = true;

    if (gs->level_current+1 == 4 && item->type == CELL_UNREFINED_COAL)
        can_take_item = false;

    if (!can_take_item) return;

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

static void slot_tick(Slot *slot) {
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
static void inventory_tick() {
    if (!gs->gui.popup) return;
    if (gs->recipes.active) return;

    if (gs->level_current == 6-1 && !gs->did_fuel_converter_tutorial) {
        gs->tutorial = *tutorial_rect(TUTORIAL_FUEL_CONVERTER_STRING, null);
        gs->did_fuel_converter_tutorial = true;
    }
    if (gs->level_current == 4-1 && !gs->did_inventory_tutorial) {
        gs->tutorial = *tutorial_rect(TUTORIAL_INVENTORY_STRING, null);
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
    item_tick(&gs->item_holding, null, input->real_mx, input->real_my, ITEM_SIZE, ITEM_SIZE);
}

// calls from gui_draw()
static void inventory_draw(int target) {
    if (gs->gui.popup_inventory_y <= 0) return;

    GUI *gui = &gs->gui;

    if (gs->gui.stored_game_scale != gs->S)
        inventory_setup_slots();

    gs->gui.stored_game_scale = gs->S;

    const f32 y = -GUI_H + gui->popup_inventory_y;

    SDL_Rect rect = {
        0, y,
        gs->S*gs->gh, GUI_H
    };

    Texture *t = &GetTexture(TEXTURE_CONVERTER_BG);
    
    f64 ratio = (f64)t->height/t->width;
    f64 width = Scale(768);
    RenderTexture(target, t, null, &(SDL_Rect){0, y+rect.h+0-width*ratio, width, width*ratio});

    for (int i = 0; i < INVENTORY_SLOT_COUNT; i++) {
        slot_draw(target, &gs->inventory.slots[i], 0, y);
    }

    Input *input = &gs->input;
    item_draw(target,
              gs->item_holding,
              input->real_mx - ITEM_SIZE/2,
              y + input->real_my - ITEM_SIZE/2,
              ITEM_SIZE,
              ITEM_SIZE);

#if 0
    RenderColor(Red(CONVERTER_LINE_COLOR),
                Green(CONVERTER_LINE_COLOR),
                Blue(CONVERTER_LINE_COLOR),
                255);
    RenderLine(target,
               0,
               y+GUI_H,
               gs->game_width,
               y+GUI_H);
#endif
}
