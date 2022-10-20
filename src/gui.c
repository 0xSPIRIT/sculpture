void converter_set_state(struct Converter *converter, enum Converter_State state) {
    converter->state = state;
    if (state == CONVERTER_OFF) {
        converter->state = CONVERTER_OFF;
        SDL_SetTextureColorMod(converter->arrow.texture, 255, 255, 255);
    }
}


bool converter_is_layout_valid(struct Converter *converter) {
    bool is_empty = true;
    for (int i = 0; i < converter->slot_count; i++) {
        if (converter->slots[i].item.type) {
            is_empty = false;
            break;
        }
    }
    
    if (is_empty) return false;
    
    return true;
}

void converter_begin_converting(void *converter_ptr) {
    struct Converter *converter = (struct Converter *) converter_ptr;

    if (!converter_is_layout_valid(converter))
        return;

    converter_set_state(converter, converter->state == CONVERTER_ON ? CONVERTER_OFF : CONVERTER_ON);
}

struct Placer *converter_get_current_placer() {
    struct Placer *placers = gs->placers;
    if (gs->current_placer == -1 || gs->current_tool != TOOL_PLACER) return NULL;
    return &placers[gs->current_placer];
}

//////////////////////////////////////////

struct Button *button_allocate(enum Button_Type type, SDL_Texture *texture, const char *tooltip_text, void (*on_pressed)(void*)) {
    struct Button *b = arena_alloc(gs->persistent_memory, 1, sizeof(struct Button));
    b->type = type;
    b->texture = texture;
    SDL_QueryTexture(texture, NULL, NULL, &b->w, &b->h);

    strcpy(b->tooltip_text, tooltip_text);
    b->on_pressed = on_pressed;
    return b;
}

void click_gui_tool_button(void *type_ptr) {
    int type = *((int*)type_ptr);
    
    struct GUI *gui = &gs->gui;

    if (gui->popup) return;

    gs->current_tool = type;
    gs->chisel_blocker_mode = 0;

    switch (gs->current_tool) {
    case TOOL_CHISEL_SMALL:
        gs->chisel = &gs->chisel_small;
        for (int i = 0; i < gs->object_count; i++)
            object_generate_blobs(i, 0);
        gs->chisel_hammer.normal_dist = gs->chisel_hammer.dist = (f32) (gs->chisel->w+2);
        break;
    case TOOL_CHISEL_MEDIUM:
        gs->chisel = &gs->chisel_medium;
        for (int i = 0; i < gs->object_count; i++)
            object_generate_blobs(i, 1);
        gs->chisel_hammer.normal_dist = gs->chisel_hammer.dist = (f32) (gs->chisel->w+4);
        break;
    case TOOL_CHISEL_LARGE:
        gs->current_tool = TOOL_CHISEL_LARGE;
        gs->chisel = &gs->chisel_large;
        for (int i = 0; i < gs->object_count; i++)
            object_generate_blobs(i, 2);
        gs->chisel_hammer.normal_dist = gs->chisel_hammer.dist = (f32) (gs->chisel->w+4);
        break;
    }

    for (int i = 0; i < TOOL_COUNT; i++) {
        if (type != i)
            gui->tool_buttons[i]->activated = false;
    }

    tooltip_reset(&gui->tooltip);
}

void button_tick(struct Button *b, void *data) {
    struct Input *input = &gs->input;
    struct GUI *gui = &gs->gui;

    int gui_input_mx = input->real_mx;
    int gui_input_my = input->real_my;

    // TODO: This is a temporary hack so that function pointers won't stop working
    //       upon reloading the DLL.
    switch (b->type) {
    case BUTTON_TYPE_CONVERTER:
        b->on_pressed = converter_begin_converting;
        break;
    case BUTTON_TYPE_TOOL_BAR:
        b->on_pressed = click_gui_tool_button;
        break;
    }

    if (gui_input_mx >= b->x && gui_input_mx < b->x+b->w &&
        gui_input_my >= b->y && gui_input_my < b->y+b->h) {

        tooltip_set_position_to_cursor(&gui->tooltip, TOOLTIP_TYPE_BUTTON);
        b->just_had_tooltip = true;

        gs->is_mouse_over_any_button = true;

        if (strlen(b->tooltip_text))
            strcpy(gui->tooltip.str[0], b->tooltip_text);

        if (input->mouse_pressed[SDL_BUTTON_LEFT]) {
            b->on_pressed(data);
        }
    } else if (b->just_had_tooltip) {
        b->just_had_tooltip = false;
        tooltip_reset(&gui->tooltip);
    }
}

void button_draw(struct Button *b) {
    struct Input *input = &gs->input;

    int gui_input_mx = input->real_mx;// / gs->S;
    int gui_input_my = input->real_my;// / gs->S;

    SDL_Rect dst = {
        b->x, b->y, b->w, b->h
    };

    if (b->activated) {
        SDL_SetTextureColorMod(b->texture, 200, 200, 200);
    } else if (gui_input_mx >= b->x && gui_input_mx < b->x+b->w &&
               gui_input_my >= b->y && gui_input_my < b->y+b->h) {
        SDL_SetTextureColorMod(b->texture, 230, 230, 230);
    } else {
        SDL_SetTextureColorMod(b->texture, 255, 255, 255);
    }
    SDL_RenderCopy(gs->renderer, b->texture, NULL, &dst);
}

void gui_message_stack_push(const char *str) {
    struct GUI *gui = &gs->gui;

    Assert(strlen(str) <= 100);
    Assert(gui->message_count <= MAX_MESSAGE_STACK);

    if (gui->message_count) {
        for (int i = 0; i < gui->message_count; i++) {
            gui->message_stack[i+1] = gui->message_stack[i];
        }
    }
    gui->message_count++;
    struct Message *msg = &gui->message_stack[0];

    strcpy(msg->str, str);
    msg->alpha = 255;
}

void gui_message_stack_tick_and_draw() {
    struct GUI *gui = &gs->gui;

    for (int i = 0; i < gui->message_count; i++) {
        struct Message *msg = &gui->message_stack[i];
    
        SDL_Color col = { 255, 255, 255, msg->alpha };
        draw_text(gs->fonts.font_consolas, msg->str, col, true, true, 0, GUI_H+gs->S*gs->gh - i*32, NULL, NULL);
    
        msg->alpha--;
        if (msg->alpha == 127) {
            gui->message_count--;
            for (int j = i; j < gui->message_count; j++) {
                gui->message_stack[j] = gui->message_stack[j+1];
            }
            i--;
        }
    }
}

void gui_init() {
    struct GUI *gui = &gs->gui;

    gui->popup_y = (f32) (gs->gh*gs->S);
    gui->popup_y_vel = 0;
    gui->popup_h = GUI_POPUP_H;
    gui->popup = 0;
    gui->popup_texture = gs->textures.popup;

    tooltip_reset(&gui->tooltip);
    gui->is_placer_active = false;

    int cum = 0;

    for (int i = 0; i < TOOL_COUNT; i++) {
        char name[128] = {0};
        get_name_from_tool(i, name);

        if (gui->tool_buttons[i] == NULL) {
            gui->tool_buttons[i] = button_allocate(BUTTON_TYPE_TOOL_BAR, gs->textures.tool_buttons[i], name, click_gui_tool_button);
        }

        gui->tool_buttons[i]->x = cum;
        gui->tool_buttons[i]->y = 0;
        gui->tool_buttons[i]->index = i;
        gui->tool_buttons[i]->activated = i == gs->current_tool;

        cum += gui->tool_buttons[i]->w;
    }

    overlay_interface_init();
}

void gui_tick() {
    struct GUI *gui = &gs->gui;
    struct Input *input = &gs->input;

    if (input->keys_pressed[SDL_SCANCODE_TAB]) {
        gui->popup = !gui->popup;
        gui->popup_y_vel = 0;
        tooltip_reset(&gui->tooltip);

        gs->current_tool = gs->previous_tool;

        // Just in case the player had reset it.
        if (gs->current_placer == -1)
            gs->current_placer = 0;
    }

    const f32 speed = 3.0f;

    if (gui->popup) {
        if (SDL_GetCursor() != gs->grabber_cursor) {
            /* SDL_SetCursor(gs->grabber_cursor); */
            /* SDL_ShowCursor(1); */
        }

        if (gui->popup_y > gs->S*gs->gh-gui->popup_h) {
            gui->popup_y_vel -= speed;
        } else {
            gui->popup_y_vel = 0;
            gui->popup_y = gs->S*gs->gh-gui->popup_h;
        }

        int was_placer_active = gui->is_placer_active;

        gui->is_placer_active = input->keys[SDL_SCANCODE_LCTRL];

        if (was_placer_active && !gui->is_placer_active) {
            tooltip_reset(&gui->tooltip);
        } else if (!was_placer_active && gui->is_placer_active) {
            struct Placer *p = converter_get_current_placer();
            p->x = input->mx;
            p->y = input->my;
        }
    } else if (gui->popup_y < gs->S*gs->gh) {
        gui->popup_y_vel += speed;
    } else {
        gui->popup_y = (f32) (gs->S*gs->gh);
        gui->popup_y_vel = 0;
    }

    if (!gui->popup) {
        /* SDL_SetCursor(normal_cursor); */
        for (int i = 0; i < TOOL_COUNT; i++) {
            button_tick(gui->tool_buttons[i], &i);
        }

        if (gs->current_tool == TOOL_OVERLAY)
            overlay_interface_tick();

        if (input->real_my >= GUI_H) {
            // tooltip_reset(&gui->tooltip);
        }
    }

    gui->popup_y += gui->popup_y_vel;
    gui->popup_y = (f32) clamp((int) gui->popup_y, (int) (gs->S*gs->gh - gui->popup_h), gs->window_height);
}

void gui_draw() {
    struct GUI *gui = &gs->gui;

    // Draw the toolbar buttons.
    SDL_Texture *old = SDL_GetRenderTarget(gs->renderer);

    SDL_SetTextureBlendMode(RenderTarget(RENDER_TARGET_GUI_TOOLBAR), SDL_BLENDMODE_BLEND);

    Assert(RenderTarget(RENDER_TARGET_GUI_TOOLBAR));
    SDL_SetRenderTarget(gs->renderer, RenderTarget(RENDER_TARGET_GUI_TOOLBAR));

    SDL_SetRenderDrawColor(gs->renderer, 0, 0, 0, 0);
    SDL_RenderClear(gs->renderer);

    SDL_SetRenderDrawColor(gs->renderer, 64, 64, 64, 255);
    SDL_Rect r = { 0, 0, gs->gw, GUI_H/gs->S };
    SDL_RenderFillRect(gs->renderer, &r);

    for (int i = 0; i < TOOL_COUNT; i++) {
        button_draw(gui->tool_buttons[i]);
    }

    SDL_Rect dst = {
        0, 0,
        gs->gw*gs->S, GUI_H
    };

    SDL_SetRenderTarget(gs->renderer, NULL);
    SDL_RenderCopy(gs->renderer, RenderTarget(RENDER_TARGET_GUI_TOOLBAR), NULL, &dst);
    SDL_SetRenderTarget(gs->renderer, old);
}

void gui_popup_draw() {
    struct GUI *gui = &gs->gui;

    SDL_Rect popup = {
        0, (int)(GUI_H + gui->popup_y),
        gs->gw*gs->S, (int)gui->popup_h
    };

    SDL_SetRenderDrawColor(gs->renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(gs->renderer, &popup);
}

bool is_cell_fuel(int type) {
    switch (type) {
    case CELL_REFINED_COAL: case CELL_UNREFINED_COAL: case CELL_LAVA:
        return true;
    }
    return false;
}

bool is_cell_stone(int type) {
    switch (type) {
    case CELL_COBBLESTONE: case CELL_MARBLE: case CELL_SANDSTONE:
    case CELL_CONCRETE: case CELL_QUARTZ: case CELL_GRANITE:
    case CELL_BASALT: case CELL_DIAMOND:
        return true;
    default:
        return false;
    }
}

// Possibilities:
//   Tiers 1, 2, or 3.
//   Returns 0 if no tier is specified.
int get_cell_tier(int type) {
    switch (type) {
    case CELL_MARBLE: case CELL_COBBLESTONE: case CELL_SANDSTONE:
        return 1;
        break;
    case CELL_CEMENT: case CELL_CONCRETE: case CELL_QUARTZ: case CELL_GLASS:
        return 2;
        break;
    case CELL_GRANITE: case CELL_BASALT: case CELL_DIAMOND: // case CELL_GOLD:
        return 3;
        break;

    case CELL_UNREFINED_COAL:
        return 1;
    case CELL_REFINED_COAL:
        return 2;
    case CELL_LAVA:
        return 3;
    }

    return 0; // 0 = not specified in any tier.
}

bool can_place_item_in_slot(int type, int slot) {
    bool can_put_fuel = false;
    
    if (slot == SLOT_FUEL) {
        can_put_fuel = is_cell_fuel(type);
    }
    
    return
        slot == SLOT_INPUT1 ||
        slot == SLOT_INPUT2 ||
        can_put_fuel;
}

bool is_mouse_in_slot(struct Slot *slot) {
    struct Input *input = &gs->input;
    
    return is_point_in_rect((SDL_Point){input->real_mx, input->real_my-GUI_H},
                            (SDL_Rect){
                                (int) (slot->converter->x + slot->x - slot->w/2),
                                (int) (slot->converter->y + slot->y - slot->h/2),
                                (int) slot->w,
                                (int) slot->h
                            });
}

bool was_mouse_in_slot(struct Slot *slot) {
    struct Input *input = &gs->input;

    return is_point_in_rect((SDL_Point){input->real_pmx,input->real_pmy-GUI_H},
                            (SDL_Rect){
                                (int) (slot->converter->x + slot->x - slot->w/2),
                                (int) (slot->converter->y + slot->y - slot->h/2),
                                (int) slot->w,
                                (int) slot->h
                            });
}

bool is_mouse_in_converter(struct Converter *converter) {
    struct Input *input = &gs->input;
    
    return is_point_in_rect((SDL_Point){
            input->real_mx,
            input->real_my-GUI_H
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
        if (input->real_my < gs->gui.popup_y && input->mouse_pressed[SDL_BUTTON_LEFT]) {
            // We pressed outside of the converter area.
            // We will now kill the holding item.
            gs->item_holding.type = 0;
            gs->item_holding.amount = 0;
        }
        return;
    }    

    if (!is_point_in_rect((SDL_Point){input->real_mx, input->real_my-GUI_H}, (SDL_Rect){x, y, w, h})) return;
    
    // From this point onwards, we know the mouse is in this item,
    // and this item is not currently being held.
    
    bool can_place_item = false;
    
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
            // Split the item into two like minecraft.
            Assert(gs->item_holding.amount == 0);
            
            const int half = item->amount/2;
            
            if (half) {
                item->amount -= half;
                gs->item_holding.type = item->type;
                gs->item_holding.amount += half;
                
                tooltip_reset(&gs->gui.tooltip);
            }
        }
    } else if (gs->gui.is_placer_active) {
        struct Placer *p = converter_get_current_placer();
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
            int amt = 0;
            const int place_speed = 6;
            
            if (p->contains_amount && (!item->type || !item->amount)) {
                item->type = p->contains_type;
                amt = place_speed;
            } else if (item->type == p->contains_type) {
                amt = place_speed;
            }
            
            if (p->contains_amount - converter->speed < 0) {
                amt = p->contains_amount;
            }
            
            item->amount += amt;
            p->contains_amount -= amt;
        }
    }
}

void slot_tick(struct Slot *slot) {
    item_tick(&slot->item,
              slot,
              (int) (slot->converter->x + slot->x - slot->w/2),
              (int) (slot->converter->y + slot->y - slot->h/2),
              (int) slot->w,
              (int) slot->h);
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
    
    SDL_Color color = (SDL_Color){255, 255, 255, 255};
    
    SDL_Surface *surf = TTF_RenderText_Solid(gs->fonts.font_bold_small, number, color);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(gs->renderer, surf);
    
    SDL_Rect dst = {
        x + w - surf->w - 1,
        y + h - surf->h - 1,
        surf->w,
        surf->h
    };
    
    SDL_SetRenderDrawColor(gs->renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(gs->renderer, &dst);
    
    SDL_RenderCopy(gs->renderer, texture, NULL, &dst);

    SDL_FreeSurface(surf);
    SDL_DestroyTexture(texture);
}

void slot_draw(struct Slot *slot) {
    struct Converter *c = slot->converter;
    SDL_Rect bounds = {
        (int) (c->x + slot->x - slot->w/2),
        (int) (c->y + slot->y - slot->h/2 + GUI_H),
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
    
    if (*slot->name) {
        SDL_Surface **surf = &gs->surfaces.slot_names[SLOT_MAX_COUNT * c->type + slot->type];
        SDL_Texture **texture = &gs->textures.slot_names[SLOT_MAX_COUNT * c->type + slot->type];
        
        if (!*surf) {
            *surf = TTF_RenderText_Blended(gs->fonts.font_small, slot->name, (SDL_Color){0, 0, 0, 255});
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

//////////////////////////////////////////

struct Converter *converter_init(int type, bool allocated) {
    struct Converter *converter = NULL;

    if (!allocated) {
        converter = arena_alloc(gs->persistent_memory, 1, sizeof(struct Converter));
    } else {
        switch (type) {
        case CONVERTER_FUEL:
            converter = gs->fuel_converter;
            break;
        case CONVERTER_MATERIAL:
            converter = gs->material_converter;
            break;
        }
    }
    
    converter->type = type;
    converter->w = (f32) (gs->window_width/2);
    converter->h = GUI_POPUP_H;
    
    converter->timer_max = 1;
    converter->timer_current = 0;
    
    switch (type) {
    case CONVERTER_MATERIAL:
        converter->slot_count = 4;

        if (!allocated) {
            converter->slots = arena_alloc(gs->persistent_memory, converter->slot_count, sizeof(struct Slot));
        }
        
        converter->slots[SLOT_INPUT1].x = converter->w/3.f;
        converter->slots[SLOT_INPUT1].y = converter->h/4.f;
        strcpy(converter->slots[SLOT_INPUT1].name, "Inp. 1");
        
        converter->slots[SLOT_INPUT2].x = 2.f*converter->w/3.f;
        converter->slots[SLOT_INPUT2].y = converter->h/4.f;
        strcpy(converter->slots[SLOT_INPUT2].name, "Inp. 2");
        
        converter->slots[SLOT_FUEL].x = 3.f*converter->w/4.f;
        converter->slots[SLOT_FUEL].y = converter->h/2.f;
        strcpy(converter->slots[SLOT_FUEL].name, "Fuel");
        
        converter->slots[SLOT_OUTPUT].x = converter->w/2.f;
        converter->slots[SLOT_OUTPUT].y = 4.f*converter->h/5.f;
        strcpy(converter->slots[SLOT_OUTPUT].name, "Output");
        
        strcpy(converter->name, "Material Converter");
        break;
    case CONVERTER_FUEL:
        converter->slot_count = 3;

        if (!allocated) {
            converter->slots = arena_alloc(gs->persistent_memory, converter->slot_count, sizeof(struct Slot));
        }
        
        converter->slots[SLOT_INPUT1].x = converter->w/3.f;
        converter->slots[SLOT_INPUT1].y = converter->h/4.f;
        strcpy(converter->slots[SLOT_INPUT1].name, "Inp. 1");
        
        converter->slots[SLOT_INPUT2].x = 2.f*converter->w/3.f;
        converter->slots[SLOT_INPUT2].y = converter->h/4.f;
        strcpy(converter->slots[SLOT_INPUT2].name, "Inp. 2");
        
        converter->slots[SLOT_OUTPUT].x = converter->w/2.f;
        converter->slots[SLOT_OUTPUT].y = 4.f*converter->h/5.f;
        strcpy(converter->slots[SLOT_OUTPUT].name, "Output");
        
        strcpy(converter->name, "Fuel Converter");
        break;
    }
    
    // Indices line up with Slot_Type enum
    // even though slot_count is variable.
    for (int i = 0; i < converter->slot_count; i++) {
        converter->slots[i].type = i;
        converter->slots[i].w = ITEM_SIZE;
        converter->slots[i].h = ITEM_SIZE;
        converter->slots[i].converter = converter;
    }
    
    converter->arrow.texture = gs->textures.converter_arrow;
    SDL_QueryTexture(converter->arrow.texture, NULL, NULL, &converter->arrow.w, &converter->arrow.h);
    
    converter->arrow.x = (int) (converter->w/2);
    converter->arrow.y = (int) (converter->h/2 + 24);
    converter->speed = 8;
    
    // Both X and Y-coordinates are updated in converter_tick.
    if (converter->go_button == NULL) {
        converter->go_button = button_allocate(BUTTON_TYPE_CONVERTER, gs->textures.convert_button, "Convert", converter_begin_converting);
    }
    converter->go_button->w = 48;
    converter->go_button->h = 48;
    
    return converter;
}

void all_converters_init() {
    bool allocated = gs->material_converter != NULL || gs->fuel_converter != NULL;
    gs->material_converter = converter_init(CONVERTER_MATERIAL, allocated);
    gs->fuel_converter = converter_init(CONVERTER_FUEL, allocated);
}

int get_number_unique_inputs(struct Item *input1, struct Item *input2) {
    int number_inputs = (input1->type != 0) + (input2->type != 0);
    int number_unique_inputs = 0;

    if (number_inputs == 1) {
        number_unique_inputs = 1;
    } else if (number_inputs == 2) {
        if (input1->type != input2->type) {
            number_unique_inputs = 2;
        } else {
            number_unique_inputs = 1;
        }
    }

    return number_unique_inputs;
}

struct Converter_Checker converter_checker(struct Item *input1, struct Item *input2) {
    Assert(input1);
    Assert(input2);

    return (struct Converter_Checker) {
        input1, input2, 0
    };
}

bool is_either_input_type(struct Converter_Checker *checker, int type, bool restart) {
    Assert(checker->input1);
    Assert(checker->input2);

    if (restart) {
        checker->current = 0;
    }

    if (checker->current == 0) {
        if (checker->input1->type == type) {
            checker->current = 2;
        } else if (checker->input2->type == type) {
            checker->current = 1;
        } else {
            return false;
        }
    } else {
        if ((checker->current == 2 && checker->input2->type != type) ||
            (checker->current == 1 && checker->input1->type != type)) {
            return false;
        }
    }

    return true;
}

bool is_either_input_tier(struct Converter_Checker *checker, int tier, bool is_fuel, bool restart) {
    Assert(checker->input1);
    Assert(checker->input2);

    if (restart) {
        checker->current = 0;
    }

    if (checker->current == 0) {
        if (get_cell_tier(checker->input1->type) == tier) {
            checker->current = 2;
        } else if (get_cell_tier(checker->input2->type) == tier) {
            checker->current = 1;
        }
    } else {
        if (is_fuel) {
            return ((checker->current == 2 && is_cell_fuel(checker->input2->type) && tier == get_cell_tier(checker->input2->type)) ||
                    (checker->current == 1 && is_cell_fuel(checker->input1->type) && tier == get_cell_tier(checker->input1->type)));
        } else {
            return ((checker->current == 2 && !is_cell_fuel(checker->input2->type) && tier == get_cell_tier(checker->input2->type)) ||
                    (checker->current == 1 && !is_cell_fuel(checker->input1->type) && tier == get_cell_tier(checker->input1->type)));
        }
    }

    return true;
}

bool is_either_input_stone(struct Converter_Checker *checker, bool restart) {
    Assert(checker->input1);
    Assert(checker->input2);

    if (restart) {
        checker->current = 0;
    }

    if (checker->current == 0) {
        if (is_cell_stone(checker->input1->type)) {
            checker->current = 2;
        } else if (is_cell_stone(checker->input2->type)) {
            checker->current = 1;
        }
    } else {
        if ((checker->current == 2 && !is_cell_stone(checker->input2->type)) ||
            (checker->current == 1 && !is_cell_stone(checker->input1->type))) {
            return false;
        }
    }

    return true;
}

// Calculate the ratio of inputs -> output.
f32 calculate_output_ratio(struct Item *input1, struct Item *input2) {
    f32 result = 0.f;
    int number_unique_inputs = get_number_unique_inputs(input1, input2);

    result = (f32) (number_unique_inputs * number_unique_inputs);

    return result;
}

int fuel_converter_convert(struct Item *input1, struct Item *input2) {
    int result_type = 0;
    int number_inputs = (input1->type != 0) + (input2->type != 0);
    int number_unique_inputs = 0;

    struct Item *input = NULL;

    if (number_inputs == 1) {
        input = input1->type ? input1 : input2;
    } else if (number_inputs == 2) {
        input = input1;
    }

    number_unique_inputs = get_number_unique_inputs(input1, input2);

    if (number_unique_inputs == 1) {
        if (input->type == CELL_DIRT ||
            input->type == CELL_SAND ||
            !is_cell_fuel(input->type) && get_cell_tier(input->type) == 1)
        {
            result_type = CELL_UNREFINED_COAL;
        }
    }
    else if (number_unique_inputs == 2) {
        struct Converter_Checker checker = converter_checker(input1, input2);

        if (is_either_input_type(&checker, CELL_UNREFINED_COAL, false) &&
            is_either_input_tier(&checker, 2, false, false))
        {
            result_type = CELL_REFINED_COAL;
        }

        else if (is_either_input_type(&checker, CELL_UNREFINED_COAL, true) &&
                 is_either_input_type(&checker, CELL_LAVA, false))
        {
            result_type = CELL_REFINED_COAL; // TODO: Higher output ratio.
        }

        else if (is_either_input_type(&checker, CELL_REFINED_COAL, true) &&
                 is_either_input_stone(&checker, false))
        {
            result_type = CELL_LAVA;
        }
    }

    return result_type;
}

int material_converter_convert(struct Item *input1, struct Item *input2, struct Item *fuel) {
    Assert(input1);
    Assert(input2);

    int result_type = 0;
    int number_inputs = (input1->type != 0) + (input2->type != 0);

    // TOOD: Turn this into number_unique_inputs? Won't this not work
    //       with that?

    struct Item *input = NULL;

    if (number_inputs == 1) {
        input = input1->type ? input1 : input2;
    } else if (number_inputs == 2) {
        input = input1;
    }
    
    switch (fuel->type) {
    case CELL_NONE:
        switch (input->type) {
        case CELL_WOOD_LOG:
            result_type = CELL_WOOD_PLANK;
            break;
        }
        break;
    case CELL_UNREFINED_COAL:
        if (number_inputs == 2) {
            struct Converter_Checker checker = converter_checker(input1, input2);

            if (is_either_input_type(&checker, CELL_COBBLESTONE, true) &&
                is_either_input_type(&checker, CELL_SAND, false))
            {
                result_type = CELL_SANDSTONE;
            }
        } else if (number_inputs == 1) {
            switch (input->type) {
            case CELL_SAND:
                result_type = CELL_GLASS;
                break;
            case CELL_DIRT:
                result_type = CELL_COBBLESTONE;
                break;
            case CELL_COBBLESTONE:
                result_type = CELL_MARBLE;
                break;
            case CELL_ICE:
                result_type = CELL_WATER;
                break;
            case CELL_WATER:
                result_type = CELL_STEAM;
                break;
            }
        }
        break;
    case CELL_REFINED_COAL:
        if (number_inputs == 1) {
            switch (input->type) {
            case CELL_DIRT:
                result_type = CELL_COBBLESTONE;
                break;
            case CELL_ICE:
                result_type = CELL_STEAM;
                break;
            }
        } else if (number_inputs == 2) {
            struct Converter_Checker checker = converter_checker(input1, input2);

            if (is_either_input_type(&checker, CELL_SANDSTONE, true) &&
                is_either_input_type(&checker, CELL_MARBLE, false))
            {
                result_type = CELL_QUARTZ;
            }
            else if (is_either_input_type(&checker, CELL_WATER, true) &&
                     is_either_input_type(&checker, CELL_COBBLESTONE, false))
            {
                result_type = CELL_CEMENT;
            }
        }

        break;
    case CELL_LAVA:
        if (number_inputs == 1) {
            switch (input->type) {
            case CELL_REFINED_COAL: case CELL_UNREFINED_COAL:
                result_type = CELL_BASALT;
                break;
            }
        } else if (number_inputs == 2) {
            struct Converter_Checker checker = converter_checker(input1, input2);

            if (is_either_input_type(&checker, CELL_QUARTZ, true) &&
                is_either_input_type(&checker, CELL_MARBLE, false))
            {
                result_type = CELL_GRANITE;
            }
            else if (is_either_input_type(&checker, CELL_BASALT, true) &&
                     is_either_input_type(&checker, CELL_GRANITE, false))
            {
                result_type = CELL_DIAMOND;
            }
        }
        break;
    }
    
    return result_type;
}

// Returns if the conversion went succesfully.
bool converter_convert(struct Converter *converter) {
    bool did_convert = false;

    int temp_output_type = 0;

    struct Item *input1 = &converter->slots[SLOT_INPUT1].item;
    struct Item *input2 = &converter->slots[SLOT_INPUT2].item;
    struct Item *output = &converter->slots[SLOT_OUTPUT].item;
    struct Item *fuel = NULL;

    if (converter->type == CONVERTER_MATERIAL) {
        fuel = &converter->slots[SLOT_FUEL].item;
    }

    int number_inputs = (input1->type != 0) + (input2->type != 0);

    if (!number_inputs) return false;
    /* if (fuel && (!fuel->type || fuel->amount == 0)) return false; */

    if (converter->type == CONVERTER_FUEL) {
        temp_output_type = fuel_converter_convert(input1, input2);
    } else if (converter->type == CONVERTER_MATERIAL) {
        temp_output_type = material_converter_convert(input1, input2, fuel);
    }

    if (!temp_output_type) return false;

    f32 output_ratio = calculate_output_ratio(input1, input2);

    bool final_conversion = false;

    // Actually remove amounts from the inputs and increase
    // or set the output.
    if (temp_output_type == output->type || output->type == 0) {
        output->type = temp_output_type;

        int amount = (int) (converter->speed * output_ratio);

        // Check if any input, when reduced by the amount,
        // gives negative amount. If so, lock the amount
        // we're reducing to by the correct amount.
        if (input1->type && input1->amount-amount < 0) {
            amount = input1->amount;
            final_conversion = true;
        }
        if (input2->type && input2->amount-amount < 0) {
            amount = input2->amount;
            final_conversion = true;
        }

        // The same for fuel, but make sure to multiple by the
        // number of inputs to compensate otherwise you'd be able
        // to use half the amount of fuel for the same output if
        // you just split the input in two.
        if (fuel && fuel->type && fuel->amount-amount < 0) {
            amount = fuel->amount * number_inputs; // *ing to compensate.
            final_conversion = true;
        }

        int amount_add = 0;
        if (input1->type) {
            input1->amount -= amount;
            amount_add += amount;
        }
        if (input2->type) {
            input2->amount -= amount;
            amount_add += amount;
        }

        if (fuel && fuel->type)
            fuel->amount -= amount_add;
        output->amount += amount_add;

        did_convert = true;
    }

    return !final_conversion || !did_convert;
}

void converter_tick(struct Converter *converter) {
    converter->arrow.y = (int) (converter->h/2 + 18);
    
    switch (converter->type) {
    case CONVERTER_MATERIAL:
        converter->x = 0;
        converter->y = gs->gui.popup_y;
        break;
    case CONVERTER_FUEL:
        converter->x = (f32) (gs->S*gs->gw/2);
        converter->y = (f32) (gs->gui.popup_y);
        break;
    }
    
    converter->go_button->x = (int) (converter->x + converter->arrow.x - 128);
    converter->go_button->y = (int) (gs->gui.popup_y + 240);
    
    for (int i = 0; i < converter->slot_count; i++) {
        struct Slot *s = &converter->slots[i];
        slot_tick(s);
    }
    
    button_tick(converter->go_button, converter);
    
    if (converter->state == CONVERTER_ON) {
        if (converter->timer_current < converter->timer_max) {
            converter->timer_current += 1;
        } else {
            converter->timer_current = 0;
            switch (converter->type) {
            case CONVERTER_MATERIAL: {
                bool did_convert = converter_convert(gs->material_converter);
                if (!did_convert) {
                    converter_set_state(gs->material_converter, CONVERTER_OFF);
                }
                break;
            }
            case CONVERTER_FUEL: {
                bool did_convert = converter_convert(gs->fuel_converter);
                if (!did_convert) {
                    converter_set_state(gs->fuel_converter, CONVERTER_OFF);
                }
                break;
            }
            }
        }
    }
    
    if (!gs->gui.popup) return;
}

void all_converters_tick() {
    if (gs->gui.popup && gs->current_tool != TOOL_PLACER) {
        gs->previous_tool = gs->current_tool;
        gs->current_tool = TOOL_PLACER;
    }
    
    if (gs->gui.popup && gs->gui.is_placer_active) {
        placer_tick(converter_get_current_placer());
    }
    
    converter_tick(gs->material_converter);
    converter_tick(gs->fuel_converter);
    
    if (!gs->gui.is_placer_active) {
        bool set_tooltip_this_frame = false;
        
        for (int i = 0; i < 2; i++) {
            struct Converter *c = i == 0 ? gs->material_converter : gs->fuel_converter;
            
            for (int j = 0; j < c->slot_count; j++) {
                if (!c->slots[j].item.type) continue;
                
                if (is_mouse_in_slot(&c->slots[j])) {
                    tooltip_set_position_to_cursor(&gs->gui.tooltip, TOOLTIP_TYPE_ITEM);
                    
                    char type[64] = {0};
                    char type_name[64] = {0};
                    char amount[64] = {0};
                    
                    get_name_from_type(c->slots[j].item.type, type_name);
                    sprintf(type, "Type: %s", type_name);
                    sprintf(amount, "Amount: %d", c->slots[j].item.amount);
                    
                    strcpy(gs->gui.tooltip.str[0], type);
                    strcpy(gs->gui.tooltip.str[1], amount);
                    
                    set_tooltip_this_frame = true;
                } else if (!set_tooltip_this_frame && gs->gui.tooltip.type == TOOLTIP_TYPE_ITEM) {
                    tooltip_reset(&gs->gui.tooltip);
                }
            }
        }
    }
    
    struct Input *input = &gs->input;
    item_tick(&gs->item_holding, NULL, input->real_mx, input->real_my, ITEM_SIZE, ITEM_SIZE);
}

void converter_draw(struct Converter *converter) {
    SDL_Rect converter_bounds = {
        (int)converter->x, (int)converter->y + GUI_H,
        (int)converter->w, (int)converter->h
    };
    
    SDL_SetRenderDrawColor(gs->renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(gs->renderer, &converter_bounds);
    
    for (int i = 0; i < converter->slot_count; i++) {
        slot_draw(&converter->slots[i]);
    }
    
    SDL_Rect arrow_dst = {
        (int) (converter->x + converter->arrow.x - converter->arrow.w / 2.0),
        (int) (converter->y + converter->arrow.y + converter->arrow.h / 2.0),
        converter->arrow.w,
        converter->arrow.h
    };
    
    // Flashing the arrow itself.
    if (converter->state == CONVERTER_ON) {
        const int period = 500; // Milliseconds.
        Uint8 a = (SDL_GetTicks()/period) % 2 == 0;
        a = a ? 255 : 190;

        SDL_SetTextureColorMod(converter->arrow.texture, a, a, a);
    }
    // Since we use the same texture for both converters,
    // we need to reset it every time as well.
    else {
        SDL_SetTextureColorMod(converter->arrow.texture, 255, 255, 255);
    }
    
    SDL_RenderCopy(gs->renderer, converter->arrow.texture, NULL, &arrow_dst);

    SDL_Surface **surf = &gs->surfaces.converter_names[converter->type];
    SDL_Texture **tex = &gs->textures.converter_names[converter->type];
    
    if (!*surf) {
        *surf = TTF_RenderText_Blended(gs->fonts.font_courier, converter->name, (SDL_Color){0, 0, 0, 255});
    }
    Assert(*surf);

    if (!*tex) {
        *tex = SDL_CreateTextureFromSurface(gs->renderer, *surf);
    }
    Assert(*tex);
    
    int margin = 8;
    SDL_Rect r = {
        (int) (converter->x + margin),
        (int) (converter->y + margin + GUI_H),
        (*surf)->w,
        (*surf)->h
    };
    
    SDL_RenderCopy(gs->renderer, *tex, NULL, &r);
    
    button_draw(converter->go_button);
}

void all_converters_draw() {
    converter_draw(gs->material_converter);
    converter_draw(gs->fuel_converter);
    
    if (gs->gui.popup && gs->gui.is_placer_active) {
        placer_draw(converter_get_current_placer(), true);
    }
    
    struct Input *input = &gs->input;
    item_draw(&gs->item_holding, input->real_mx - ITEM_SIZE/2, input->real_my - ITEM_SIZE/2, ITEM_SIZE, ITEM_SIZE);
}
