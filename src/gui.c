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
    
    if (converter->state == CONVERTER_OFF) {
        save_state_to_next();
    }
    
    converter_set_state(converter, converter->state == CONVERTER_ON ? CONVERTER_OFF : CONVERTER_ON);
}

//////////////////////////////////////////

struct Button *button_allocate(enum Button_Type type, SDL_Texture *texture, const char *tooltip_text, void (*on_pressed)(void*)) {
    struct Button *b = PushSize(gs->persistent_memory, sizeof(struct Button));
    b->type = type;
    b->texture = texture;
    b->disabled = false;
    SDL_QueryTexture(texture, NULL, NULL, &b->w, &b->h);
    
    strcpy(b->tooltip_text, tooltip_text);
    b->on_pressed = on_pressed;
    return b;
}

void click_gui_tool_button(void *type_ptr) {
    int type = *(int*)type_ptr;
    
    struct GUI *gui = &gs->gui;
    
    if (gui->popup) return;
    
    int p_tool = gs->current_tool;
    
    gs->current_tool = type;
    gs->chisel_blocker_mode = 0;
    
    switch (gs->current_tool) {
        case TOOL_CHISEL_SMALL: {
            gs->chisel = &gs->chisel_small;
            for (int i = 0; i < gs->object_count; i++)
                object_generate_blobs(i, 0);
            gs->chisel_hammer.normal_dist = gs->chisel_hammer.dist = (f32) (gs->chisel->w+2);
            break;
        }
        case TOOL_CHISEL_MEDIUM: {
            gs->chisel = &gs->chisel_medium;
            for (int i = 0; i < gs->object_count; i++)
                object_generate_blobs(i, 1);
            gs->chisel_hammer.normal_dist = gs->chisel_hammer.dist = (f32) (gs->chisel->w+4);
            break;
        }
        case TOOL_CHISEL_LARGE: {
            gs->current_tool = TOOL_CHISEL_LARGE;
            gs->chisel = &gs->chisel_large;
            for (int i = 0; i < gs->object_count; i++)
                object_generate_blobs(i, 2);
            gs->chisel_hammer.normal_dist = gs->chisel_hammer.dist = (f32) (gs->chisel->w+4);
            break;
        }
        case TOOL_OVERLAY: {
            gs->overlay.show = !gs->overlay.show;
            gs->current_tool = p_tool;
            return;
        }
    }
    
    gui->tool_buttons[type]->active = true;
    
    for (int i = 0; i < TOOL_COUNT; i++) {
        if (type != i)
            gui->tool_buttons[i]->active = false;
    }
    
    tooltip_reset(&gui->tooltip);
}

void button_tick(struct Button *b, void *data) {
    struct Input *input = &gs->input;
    struct GUI *gui = &gs->gui;
    
    int gui_input_mx = input->real_mx;
    int gui_input_my = input->real_my;
    
    // TODO: This is a hack so that function pointers won't stop working
    //       upon reloading the DLL... Honestly why don't we just do it
    //       this way normally? Seems to work great.
    switch (b->type) {
        case BUTTON_TYPE_CONVERTER:         b->on_pressed = converter_begin_converting; break;
        case BUTTON_TYPE_TOOL_BAR:          b->on_pressed = click_gui_tool_button;      break;
        //case BUTTON_TYPE_OVERLAY_INTERFACE: b->on_pressed = click_overlay_interface;    break;
        case BUTTON_TYPE_TUTORIAL:          b->on_pressed = tutorial_rect_close;        break;
    }
    
    if (b->disabled) return;
    
    if (gui_input_mx >= b->x && gui_input_mx < b->x+b->w &&
        gui_input_my >= b->y && gui_input_my < b->y+b->h) {
        
        if (*b->tooltip_text)
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
    
    if (b->disabled) {
        SDL_SetTextureColorMod(b->texture, 75, 75, 75);
    } else if (b->active) {
        SDL_SetTextureColorMod(b->texture, 200, 200, 200);
    } else if (b->highlighted) {
        SDL_SetTextureColorMod(b->texture, 128, 128, 255);
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

void gui_message_stack_tick_and_draw(void) {
    struct GUI *gui = &gs->gui;
    
    for (int i = 0; i < gui->message_count; i++) {
        struct Message *msg = &gui->message_stack[i];
        
        SDL_Color col = { 255, 255, 255, msg->alpha };
        draw_text(gs->fonts.font_consolas, msg->str, col, (SDL_Color){0, 0, 0, 255}, true, true, 0, GUI_H+gs->S*gs->gh - i*32, NULL, NULL);
        
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

void gui_init(void) {
    struct GUI *gui = &gs->gui;
    
    gui->popup_y = (f32) (gs->gh*gs->S);
    gui->popup_y_vel = 0;
    gui->popup_h = GUI_POPUP_H;
    gui->popup = 0;
    gui->popup_texture = gs->textures.popup;
    
    tooltip_reset(&gui->tooltip);
    
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
        gui->tool_buttons[i]->active = i == gs->current_tool;
        
        cum += gui->tool_buttons[i]->w;
    }
    
    //overlay_interface_init();
}

void gui_tick(void) {
    struct GUI *gui = &gs->gui;
    struct Input *input = &gs->input;
    
    if (input->keys_pressed[SDL_SCANCODE_TAB] && 
        gs->levels[gs->level_current].state == LEVEL_STATE_PLAY &&
        gs->level_current >= 3-1) 
    {
        gui->popup = !gui->popup;
        gui->popup_y_vel = 0;
        tooltip_reset(&gui->tooltip);
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
        
        //if (gs->current_tool == TOOL_OVERLAY)
        //overlay_interface_tick();
        
        if (input->real_my >= GUI_H) {
            // tooltip_reset(&gui->tooltip);
        }
    }
    
    gui->popup_y += gui->popup_y_vel;
    gui->popup_y = (f32) clamp((int) gui->popup_y, (int) (gs->S*gs->gh - gui->popup_h), gs->window_height);
}

// Gives the amount of each cell type there are
// in an array of cells.
void profile_array(struct Cell *desired,
                   char out[64][CELL_TYPE_COUNT],
                   int *count) 
{
    int counts[CELL_TYPE_COUNT] = {0};
    
    for (int i = 0; i < gs->gw*gs->gh; i++) {
        if (desired[i].type != 0) {
            counts[desired[i].type]++;
        }
    }
    
    for (int i = 0; i < CELL_TYPE_COUNT; i++) {
        if (!counts[i]) continue;
        
        char name[64];
        get_name_from_type(i, name);
        
        if (gs->level_current == 10-1) {
            sprintf(out[(*count)++], "  %-15s???", name);
        } else if (gs->overlay.changes.index < gs->overlay.changes.count-1) {
            sprintf(out[(*count)++], "  %-15s%d??", name, counts[i]);
        } else {
            sprintf(out[(*count)++], "  %-15s%d", name, counts[i]);
        }
    }
}

void gui_draw_profile() {
    struct Level *level = &gs->levels[gs->level_current];
    int count = 0;
    
    struct Cell *overlay_grid = PushArray(gs->transient_memory, gs->gw*gs->gh, sizeof(struct Cell));
    
    for (int i = 0; i < gs->gw*gs->gh; i++) {
        overlay_grid[i].type = gs->overlay.grid[i];
    }
    
    profile_array(overlay_grid, level->profile_lines, &count);
    
    int ah = 0;
    draw_text(gs->fonts.font, "Required Amounts:", (SDL_Color){255, 255, 0, 255}, BLACK, false, false, 50, GUI_H+50, NULL, &ah);
    
    int c = ah;
    for (int i = 0; i < count; i++) {
        int h;
        draw_text(gs->fonts.font, level->profile_lines[i], WHITE, BLACK, false, false, 50, GUI_H+50+c, NULL, &h);
        c += h;
    }
}

void gui_draw(void) {
    struct GUI *gui = &gs->gui;
    
    // Draw the toolbar buttons.
    SDL_Texture *old = SDL_GetRenderTarget(gs->renderer);
    
    SDL_SetTextureBlendMode(RenderTarget(RENDER_TARGET_GUI_TOOLBAR), SDL_BLENDMODE_BLEND);
    
    Assert(RenderTarget(RENDER_TARGET_GUI_TOOLBAR));
    SDL_SetRenderTarget(gs->renderer, RenderTarget(RENDER_TARGET_GUI_TOOLBAR));
    
    SDL_SetRenderDrawColor(gs->renderer, 0, 0, 0, 0);
    SDL_RenderClear(gs->renderer);
    
    // Tool bar
    if (!gs->gui.popup) {
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
    }
    
    SDL_SetRenderTarget(gs->renderer, old);
}

void converter_draw(struct Converter *converter) {
    if (converter->state == CONVERTER_INACTIVE)
        return;
    
    
    SDL_SetRenderDrawColor(gs->renderer, 0, 0, 0, 255);
    SDL_RenderDrawLine(gs->renderer, converter->x+converter->w,
                       converter->y+GUI_H, converter->x+converter->w,
                       converter->y+GUI_H+converter->h);
    
    for (int i = 0; i < converter->slot_count; i++) {
        slot_draw(converter->slots[i], converter->x, converter->y);
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
        *surf = TTF_RenderText_LCD(gs->fonts.font_courier,
                                   converter->name, 
                                   (SDL_Color){0, 0, 0, 255},
                                   (SDL_Color){200, 200, 200, 255});
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

void all_converters_draw(void) {
    converter_draw(gs->material_converter);
    converter_draw(gs->fuel_converter);    
}

void gui_popup_draw(void) {
    struct GUI *gui = &gs->gui;
    
    SDL_Rect popup = {
        0, (int)(GUI_H + gui->popup_y),
        gs->gw*gs->S, (int)gui->popup_h
    };
    
    SDL_SetRenderDrawColor(gs->renderer, 235, 235, 235, 255);
    SDL_RenderFillRect(gs->renderer, &popup);
    
    int w, h;
    SDL_QueryTexture(gs->textures.tab, NULL, NULL, &w, &h);
    
    SDL_Rect bar = {
        0, popup.y,
        gs->gw*gs->S, (int)36
    };
    
    Uint8 r = 200, g = 200, b = 200;
    
    SDL_SetRenderDrawColor(gs->renderer, r, g, b, 255);
    SDL_RenderFillRect(gs->renderer, &bar);
    
    SDL_Rect tab_icon = {
        gs->gw*gs->S - 128, (int)(GUI_H + gui->popup_y) - h,
        w, h
    };
    
    SDL_SetTextureAlphaMod(gs->textures.tab, 127);
    
    if (gs->level_current >= 3-1)
        SDL_RenderCopy(gs->renderer, gs->textures.tab, NULL, &tab_icon);
    
    all_converters_draw();
    inventory_draw();
    conversions_gui_draw();
}

bool is_cell_stone(int type) {
    switch (type) {
        case CELL_COBBLESTONE: case CELL_MARBLE: case CELL_SANDSTONE:
        case CELL_CONCRETE: case CELL_QUARTZ: case CELL_GRANITE:
        case CELL_BASALT: case CELL_DIAMOND: {
            return true;
        }
        default: return false;
    }
}

// Possibilities:
//   Tiers 1, 2, or 3.
//   Returns 0 if no tier is specified.
int get_cell_tier(int type) {
    switch (type) {
        case CELL_MARBLE: case CELL_COBBLESTONE: case CELL_SANDSTONE: {
            return 1;
        }
        
        case CELL_CEMENT: case CELL_CONCRETE: case CELL_QUARTZ:
        case CELL_GLASS: {
            return 2;
        }
        
        case CELL_GRANITE: case CELL_BASALT: case CELL_DIAMOND: {
            return 3;
        }
        
        case CELL_UNREFINED_COAL: return 1;
        case CELL_REFINED_COAL: return 2;
        case CELL_LAVA: return 3;
    }
    
    return 0; // 0 = not specified in any tier.
}

//////////////////////////////////////////

void auto_set_material_converter_slots(struct Converter *converter) {
    int level = gs->level_current;
    
    switch (level+1) {
        case 3: {
            converter->slots[SLOT_FUEL].item = (struct Item)
            {
                .type = CELL_UNREFINED_COAL,
                .amount = 179
            };
            break;
        }
        case 9: {
            converter->slots[SLOT_FUEL].item = (struct Item)
            {
                .type = CELL_UNREFINED_COAL,
                .amount = 2000
            };
        }
        default: {
            break;
        }
    }
}

struct Converter *converter_init(int type, bool allocated) {
    struct Converter *converter = NULL;
    
    if (!allocated) {
        converter = PushSize(gs->persistent_memory, sizeof(struct Converter));
    } else {
        switch (type) {
            case CONVERTER_FUEL:     converter = gs->fuel_converter;     break;
            case CONVERTER_MATERIAL: converter = gs->material_converter; break;
        }
    }
    
    converter->type = type;
    converter->w = (f32) (gs->window_width/2);
    converter->h = GUI_POPUP_H;
    
    converter->timer_max = 1;
    converter->timer_current = 0;
    
    if (type == CONVERTER_FUEL && gs->level_current+1 <= 4) {
        converter->state = CONVERTER_INACTIVE;
    } else {
        converter->state = CONVERTER_OFF;
    }
    
    switch (type) {
        case CONVERTER_MATERIAL: {
            converter->slot_count = 4;
            
            if (!allocated) {
                converter->slots = PushArray(gs->persistent_memory, converter->slot_count, sizeof(struct Slot));
            } else {
                memset(&converter->slots[SLOT_INPUT1], 0, sizeof(struct Slot));
                memset(&converter->slots[SLOT_INPUT2], 0, sizeof(struct Slot));
                memset(&converter->slots[SLOT_FUEL],   0, sizeof(struct Slot));
                memset(&converter->slots[SLOT_OUTPUT], 0, sizeof(struct Slot));
            }
            
            // NOTE: Converter slot positions are in global space,
            //       but the converter itself isn't. Yes, it's
            //       unnecessarily confusing.
            
            converter->slots[SLOT_INPUT1].x = converter->w/3.f;
            converter->slots[SLOT_INPUT1].y = GUI_H + converter->h/4.f;
            strcpy(converter->slots[SLOT_INPUT1].name, "Inp. 1");
            
            converter->slots[SLOT_INPUT2].x = 2.f*converter->w/3.f;
            converter->slots[SLOT_INPUT2].y = GUI_H + converter->h/4.f;
            strcpy(converter->slots[SLOT_INPUT2].name, "Inp. 2");
            
            converter->slots[SLOT_FUEL].x = 3.f*converter->w/4.f;
            converter->slots[SLOT_FUEL].y = GUI_H + converter->h/2.f;
            strcpy(converter->slots[SLOT_FUEL].name, "Fuel");
            
            converter->slots[SLOT_OUTPUT].x = converter->w/2.f;
            converter->slots[SLOT_OUTPUT].y = GUI_H + 4.f*converter->h/5.f;
            strcpy(converter->slots[SLOT_OUTPUT].name, "Output");
            
            strcpy(converter->name, "Material Converter");
            
            auto_set_material_converter_slots(converter);
            break;
        }
        
        case CONVERTER_FUEL: {
            converter->slot_count = 3;
            
            if (!allocated) {
                converter->slots = PushArray(gs->persistent_memory, converter->slot_count, sizeof(struct Slot));
            } else {
                memset(&converter->slots[SLOT_INPUT1], 0, sizeof(struct Slot));
                memset(&converter->slots[SLOT_INPUT2], 0, sizeof(struct Slot));
                memset(&converter->slots[SLOT_OUTPUT], 0, sizeof(struct Slot));
            }
            
            converter->slots[SLOT_INPUT1].x = converter->w/3.f;
            converter->slots[SLOT_INPUT1].y = GUI_H + converter->h/4.f;
            strcpy(converter->slots[SLOT_INPUT1].name, "Inp. 1");
            
            converter->slots[SLOT_INPUT2].x = 2.f*converter->w/3.f;
            converter->slots[SLOT_INPUT2].y = GUI_H + converter->h/4.f;
            strcpy(converter->slots[SLOT_INPUT2].name, "Inp. 2");
            
            converter->slots[SLOT_OUTPUT].x = converter->w/2.f;
            converter->slots[SLOT_OUTPUT].y = GUI_H + 4.f*converter->h/5.f;
            strcpy(converter->slots[SLOT_OUTPUT].name, "Output");
            
            strcpy(converter->name, "Fuel Converter");
            break;
        }
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

void all_converters_init(void) {
    bool allocated = gs->material_converter != NULL || gs->fuel_converter != NULL;
    gs->material_converter = converter_init(CONVERTER_MATERIAL, allocated);
    gs->fuel_converter = converter_init(CONVERTER_FUEL, allocated);
    
    switch (gs->level_current) {
        case 2: {
            break;
        }
    }
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

int fuel_converter_convert(struct Item *input1, struct Item *input2) {
    int result_type = 0;
    int number_inputs = (input1->type != 0) + (input2->type != 0);
    int number_unique_inputs = 0;
    
    struct Item *input = NULL;
    
    // We simply don't allow this.
    if (input1->type == input2->type) {
        return 0;
    }
    
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
    
    int result = 0;
    int number_inputs = (input1->type != 0) + (input2->type != 0);
    
    // We simply don't allow this.
    if (input1->type == input2->type) {
        return 0;
    }
    
    struct Item *input = NULL;
    
    if (number_inputs == 1) {
        input = input1->type ? input1 : input2;
        if (input->type == 0 || input->amount == 0) 
            return 0;
    } else if (number_inputs == 2) {
        input = input1;
    }
    
    switch (fuel->type) {
        case CELL_NONE: {
            if (number_inputs == 1) {
                switch (input->type) {
                    case CELL_WOOD_LOG: result = CELL_WOOD_PLANK; break;
                    case CELL_STEAM:    result = CELL_WATER;      break;
                    case CELL_WATER:    result = CELL_ICE;        break;
                }
            }
            break;
        }
        case CELL_UNREFINED_COAL: {
            if (number_inputs == 2) {
                struct Converter_Checker checker = converter_checker(input1, input2);
                
                if (is_either_input_type(&checker, CELL_COBBLESTONE, true) &&
                    is_either_input_type(&checker, CELL_SAND, false))
                {
                    result = CELL_SANDSTONE;
                }
            } else if (number_inputs == 1) {
                switch (input->type) {
                    case CELL_SAND:        result = CELL_GLASS;       break;
                    case CELL_DIRT:        result = CELL_COBBLESTONE; break;
                    case CELL_COBBLESTONE: result = CELL_MARBLE;      break;
                    case CELL_ICE:         result = CELL_WATER;       break;
                    case CELL_WATER:       result = CELL_STEAM;       break;
                }
            }
            break;
        }
        case CELL_REFINED_COAL: {
            if (number_inputs == 1) {
                switch (input->type) {
                    case CELL_DIRT:        result = CELL_COBBLESTONE; break;
                    case CELL_ICE:         result = CELL_STEAM;       break;
                    case CELL_COBBLESTONE: result = CELL_MARBLE;      break;
                }
            } else if (number_inputs == 2) {
                struct Converter_Checker checker = converter_checker(input1, input2);
                
                if (is_either_input_type(&checker, CELL_SANDSTONE, true) &&
                    is_either_input_type(&checker, CELL_MARBLE, false))
                {
                    result = CELL_QUARTZ;
                }
                else if (is_either_input_type(&checker, CELL_WATER, true) &&
                         is_either_input_type(&checker, CELL_COBBLESTONE, false))
                {
                    result = CELL_CEMENT;
                }
            }
            
            break;
        }
        case CELL_LAVA: {
            if (number_inputs == 1) {
                switch (input->type) {
                    case CELL_REFINED_COAL: case CELL_UNREFINED_COAL: {
                        result = CELL_BASALT;
                        break;
                    }
                    case CELL_COBBLESTONE: {
                        result = CELL_MARBLE;
                        break;
                    }
                }
            } else if (number_inputs == 2) {
                struct Converter_Checker checker = converter_checker(input1, input2);
                
                if (is_either_input_type(&checker, CELL_QUARTZ, true) &&
                    is_either_input_type(&checker, CELL_MARBLE, false))
                {
                    result = CELL_GRANITE;
                }
                else if (is_either_input_type(&checker, CELL_BASALT, true) &&
                         is_either_input_type(&checker, CELL_GRANITE, false))
                {
                    result = CELL_DIAMOND;
                }
            }
            break;
        }
    }
    
    return result;
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
    if (input1->type == input2->type) return false;
    
    /* if (fuel && (!fuel->type || fuel->amount == 0)) return false; */
    
    if (converter->type == CONVERTER_FUEL) {
        temp_output_type = fuel_converter_convert(input1, input2);
    } else if (converter->type == CONVERTER_MATERIAL) {
        temp_output_type = material_converter_convert(input1, input2, fuel);
    }
    
    if (!temp_output_type) return false;
    
    bool final_conversion = false;
    
    // Actually remove amounts from the inputs and increase
    // or set the output.
    if (temp_output_type == output->type || output->type == 0) {
        output->type = temp_output_type;
        
        int amount = converter->speed;
        
        // Check if any input, when reduced by the amount,
        // gives negative amount. If so, lock the amount
        // we're reducing to by the minimum amount.
        if (input1->type && input1->amount-amount < 0) {
            final_conversion = true;
        }
        if (input2->type && input2->amount-amount < 0) {
            final_conversion = true;
        }
        
        // Same for fuel
        if (fuel && fuel->type && fuel->amount-1 < 0) {
            final_conversion = true;
        }
        
        if (final_conversion) {
            // Find the lowest value from the inputs and fuel.
            amount = 999999999;
            for (int i = SLOT_INPUT1; i <= SLOT_FUEL; i++) {
                if (i == SLOT_OUTPUT) continue;
                if (fuel && i == SLOT_FUEL) continue;
                
                if (converter->slots[i].item.type && converter->slots[i].item.amount < amount) {
                    amount = converter->slots[i].item.amount;
                }
            }
        }
        
        if (input1->type) {
            input1->amount -= amount;
        }
        if (input2->type) {
            input2->amount -= amount;
        }
        
        if (fuel && fuel->type) {
            fuel->amount--;
        }
        output->amount += amount;
        
        did_convert = true;
    }
    
    return !final_conversion || !did_convert;
}

void converter_tick(struct Converter *converter) {
    converter->arrow.y = (int) (converter->h/2 + 18);
    
    switch (converter->type) {
        case CONVERTER_MATERIAL: {
            converter->x = 0;
            converter->y = gs->gui.popup_y;
            break;
        }
        case CONVERTER_FUEL: {
            converter->x = (f32) (gs->S*gs->gw/2);
            converter->y = (f32) (gs->gui.popup_y);
            break;
        }
    }
    
    converter->go_button->x = (int) (converter->x + converter->arrow.x - 128);
    converter->go_button->y = (int) (gs->gui.popup_y + 240);
    
    if (converter->state == CONVERTER_INACTIVE)
        return;
    
    for (int i = 0; i < converter->slot_count; i++) {
        slot_tick(&converter->slots[i]);
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

void all_converters_tick(void) {
    // Grey out the buttons for each converter
    // if no conversions are possible right now.
    
    if (0 == material_converter_convert(&gs->material_converter->slots[SLOT_INPUT1].item,
                                        &gs->material_converter->slots[SLOT_INPUT2].item,
                                        &gs->material_converter->slots[SLOT_FUEL].item)) {
        gs->material_converter->go_button->disabled = true;
    } else {
        gs->material_converter->go_button->disabled = false;
    }
    
    if (0 == fuel_converter_convert(&gs->fuel_converter->slots[SLOT_INPUT1].item,
                                    &gs->fuel_converter->slots[SLOT_INPUT2].item)) 
    {
        gs->fuel_converter->go_button->disabled = true;
    } else {
        gs->fuel_converter->go_button->disabled = false;
    }
    
    converter_tick(gs->material_converter);
    converter_tick(gs->fuel_converter);
    
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
                
                gs->gui.tooltip.set_this_frame = true;
            } else if (!gs->gui.tooltip.set_this_frame && gs->gui.tooltip.type == TOOLTIP_TYPE_ITEM) {
                tooltip_reset(&gs->gui.tooltip);
            }
        }
    }
}