static Button *button_allocate(Button_Type type, Texture *texture, const char *tooltip_text, void (*on_pressed)(void*)) {
    Button *b = PushSize(gs->persistent_memory, sizeof(Button));
    b->type = type;
    b->texture = texture;
    b->disabled = false;

    b->index=-1;

    b->w = b->texture->width;
    b->h = b->texture->height;

    strcpy(b->tooltip_text, tooltip_text);
    b->on_pressed = on_pressed;
    return b;
}

static void tool_button_set_disabled(int level) {
    Button **tools = gs->gui.tool_buttons;

    //tools[TOOL_BLOCKER]->disabled = true;

    bool is_leeway_compared = compare_cells_to_int(gs->grid, gs->overlay.grid, COMPARE_LEEWAY);
    bool is_exactly = compare_cells_to_int(gs->grid, gs->overlay.grid, 0);

    if (is_exactly && !gs->levels[level].done) {
        Mix_PlayChannel(AUDIO_CHANNEL_GUI, gs->audio.sprinkle, 0);
        gs->levels[level].done = true;
    }

    if (is_leeway_compared) {
        tools[TOOL_FINISH_LEVEL]->highlighted = true;
    } else {
        tools[TOOL_FINISH_LEVEL]->highlighted = false;
    }

    switch (level+1) {
        case 1: {
            if (!gs->gui.tool_buttons[TOOL_OVERLAY]->highlighted) {
                if (gs->chisel_small.num_times_chiseled == 0)
                    tools[TOOL_CHISEL_SMALL]->highlighted = true;
                else
                    tools[TOOL_CHISEL_SMALL]->highlighted = false;
            }

            if (gs->chisel_small.num_times_chiseled < 10) {
                tools[TOOL_CHISEL_MEDIUM]->disabled = true;
                tools[TOOL_CHISEL_LARGE]->disabled = true;
            } else {
                tools[TOOL_CHISEL_MEDIUM]->disabled = false;
                tools[TOOL_CHISEL_LARGE]->disabled = false;
                if (!gs->level1_set_highlighted) {
                    gs->level1_set_highlighted = true;
                    tools[TOOL_CHISEL_MEDIUM]->highlighted = true;
                    tools[TOOL_CHISEL_LARGE]->highlighted = true;
                }
            }
        }
        case 2: {
            //tools[TOOL_DELETER]->disabled = true;
            tools[TOOL_PLACER]->disabled = true;
            break;
        }
        case 3: {
            tools[TOOL_PLACER]->disabled = true;
            break;
        }
        default: {
            for (int i = 0; i < TOOL_COUNT; i++)
                tools[i]->disabled = false;
        }
    }
}

static void click_gui_tool_button(void *type_ptr) {
    int type = *(int*)type_ptr;

    GUI *gui = &gs->gui;

    if (gui->popup) return;

    int p_tool = gs->current_tool;

    gs->current_tool = type;

    gui->tool_buttons[type]->highlighted = false;

    switch (gs->current_tool) {
        case TOOL_CHISEL_SMALL: {
            gs->chisel = &gs->chisel_small;
            break;
        }
        case TOOL_CHISEL_MEDIUM: {
            gs->chisel = &gs->chisel_medium;
            break;
        }
        case TOOL_CHISEL_LARGE: {
            gs->current_tool = TOOL_CHISEL_LARGE;
            gs->chisel = &gs->chisel_large;
            break;
        }
        case TOOL_OVERLAY: {
            gs->overlay.show = !gs->overlay.show;
            gs->current_tool = p_tool;
            return;
        }
        case TOOL_DESTROY: {
            save_state_to_next();
            for (int i = 0; i < gs->gw*gs->gh; i++) {
                if (gs->grid[i].type) {
                    add_item_to_inventory_slot(gs->grid[i].type, 1);
                    emit_dust_explosion(gs->grid[i].type, i%gs->gw, i/gs->gw, 1);
                }
                gs->grid[i].type = gs->grid[i].object = 0;
            }
            
            gs->current_tool = p_tool;
            return;
        }
        case TOOL_FINISH_LEVEL: {
            if (gs->level_current+1 != -1 ||
                (gs->level_current+1 == 1 &&
                 compare_cells(gs->grid, gs->levels[gs->level_current].desired_grid)))
            {
                level_set_state(gs->level_current, LEVEL_STATE_OUTRO);
            }
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

static void button_tick(Button *b, void *data) {
    Input *input = &gs->input;
    GUI *gui = &gs->gui;

    int gui_input_mx = input->real_mx;
    int gui_input_my = input->real_my;

    // This is a hack so that function pointers won't stop working
    // upon reloading the DLL... Honestly why don't we just do it
    // this way normally? Seems to work great.
    switch (b->type) {
        case BUTTON_CONVERTER:     b->on_pressed = converter_begin_converting; break;
        case BUTTON_TOOL_BAR:      b->on_pressed = click_gui_tool_button;      break;
        case BUTTON_TUTORIAL:      b->on_pressed = tutorial_rect_close;        break;
        case BUTTON_EOL_POPUP_CONFIRM:     b->on_pressed = end_of_level_popup_confirm_confirm;  break;
        case BUTTON_EOL_POPUP_CANCEL:      b->on_pressed = end_of_level_popup_confirm_cancel;   break;
        case BUTTON_RESTART_POPUP_CONFIRM: b->on_pressed = restart_popup_confirm_confirm;  break;
        case BUTTON_RESTART_POPUP_CANCEL:  b->on_pressed = restart_popup_confirm_cancel;   break;
    }

    if (b->disabled) return;

    if (gui_input_mx >= b->x && gui_input_mx < b->x+b->w &&
        gui_input_my >= b->y && gui_input_my < b->y+b->h) {

        if (*b->tooltip_text) {
            tooltip_set_position_to_cursor(&gui->tooltip, TOOLTIP_TYPE_BUTTON);
            gui->tooltip.preview = b->preview;
        }

        b->just_had_tooltip = true;

        gs->is_mouse_over_any_button = true;

        if (strlen(b->tooltip_text))
            strcpy(gui->tooltip.str[0], b->tooltip_text);

        if (input->mouse_pressed[SDL_BUTTON_LEFT]) {
            Mix_HaltChannel(AUDIO_CHANNEL_GUI);
            Mix_PlayChannel(AUDIO_CHANNEL_GUI, gs->audio.accept, 0);
            b->on_pressed(data);

            // Eat the input
            input->mouse = input->mouse & !(input->mouse & SDL_BUTTON_LEFT);
            input->mouse_pressed[SDL_BUTTON_LEFT] = 0;
        }
    } else if (b->just_had_tooltip) {
        b->just_had_tooltip = false;
        tooltip_reset(&gui->tooltip);
    }
}

static void button_draw_prefer_color(int target, Button *b, SDL_Color color) {
    Input *input = &gs->input;

    int gui_input_mx = input->real_mx;// / gs->S;
    int gui_input_my = input->real_my;// / gs->S;

    if (b->index != -1) {
        b->w = b->h = GUI_H;
        b->x = b->index * b->w;
    }

    SDL_Rect dst = {
        b->x, b->y, b->w, b->h
    };

    if (b->disabled) {
        RenderTextureColorMod(b->texture, 90, 90, 90);
    } else if (b->active) {
        RenderTextureColorMod(b->texture, 200, 200, 200);
    } else if (b->highlighted) {
        f64 c = (sin(SDL_GetTicks()/100.0)+1)/2.0;
        c *= 128;
        RenderTextureColorMod(b->texture, 255-c, 255, 255-c);
    } else if (gui_input_mx >= b->x && gui_input_mx < b->x+b->w &&
               gui_input_my >= b->y && gui_input_my < b->y+b->h) {
        RenderTextureColorMod(b->texture, 230, 230, 230);
    } else {
        RenderTextureColorMod(b->texture, color.r, color.g, color.b);
    }

    RenderTexture(target,
                  b->texture,
                  NULL,
                  &dst);
}

static void button_draw(int target, Button *b) {
    button_draw_prefer_color(target, b, (SDL_Color){255,255,255,255});
}

static void gui_message_stack_push(const char *str) {
    GUI *gui = &gs->gui;

    Assert(strlen(str) <= 100);
    Assert(gui->message_count <= MAX_MESSAGE_STACK);

    if (gui->message_count) {
        for (int i = 0; i < gui->message_count; i++) {
            gui->message_stack[i+1] = gui->message_stack[i];
        }
    }
    gui->message_count++;
    Message *msg = &gui->message_stack[0];

    strcpy(msg->str, str);
    msg->alpha = 255;
}

static void gui_init(void) {
    GUI *gui = &gs->gui;

    gui->popup_y = (f32) (gs->gh*gs->S);
    gui->popup_y_vel = 0;
    gui->popup = 0;
    gui->popup_texture = &GetTexture(TEXTURE_POPUP);

    gs->gui.eol_popup_confirm = popup_confirm_init("Are you satisfied with this result?",
                                                   BUTTON_EOL_POPUP_CONFIRM,
                                                   BUTTON_EOL_POPUP_CANCEL,
                                                   end_of_level_popup_confirm_confirm,
                                                   end_of_level_popup_confirm_cancel);
    gs->gui.restart_popup_confirm = popup_confirm_init("Restart level and lose all progress?",
                                                       BUTTON_RESTART_POPUP_CONFIRM,
                                                       BUTTON_RESTART_POPUP_CANCEL,
                                                       restart_popup_confirm_confirm,
                                                       restart_popup_confirm_cancel);

    tooltip_reset(&gui->tooltip);

    int cum = 0;

    for (int i = 0; i < TOOL_COUNT; i++) {
        char name[128] = {0};
        get_name_from_tool(i, name);

        if (gui->tool_buttons[i] == NULL) {
            gui->tool_buttons[i] = button_allocate(BUTTON_TOOL_BAR,
                                                   &GetTexture(TEXTURE_TOOL_BUTTONS + i),
                                                   name,
                                                   click_gui_tool_button);
            gui->tool_buttons[i]->w = gui->tool_buttons[i]->h = GUI_H;
            SDL_SetTextureScaleMode(gui->tool_buttons[i]->texture->handle, 1); // filering
            if (gs->tool_previews[i].length)
                gui->tool_buttons[i]->preview = &gs->tool_previews[i];
        }

        gui->tool_buttons[i]->x = cum;
        gui->tool_buttons[i]->y = 0;
        gui->tool_buttons[i]->index = i;
        gui->tool_buttons[i]->active = i == gs->current_tool;

        cum += gui->tool_buttons[i]->w;
    }

    //overlay_interface_init();
}

static void gui_tick(void) {
    if (gs->levels[gs->level_current].state != LEVEL_STATE_PLAY)
        return;

    GUI *gui = &gs->gui;
    Input *input = &gs->input;

    tool_button_set_disabled(gs->level_current);

    if (input->keys_pressed[SDL_SCANCODE_TAB] &&
        gs->levels[gs->level_current].state == LEVEL_STATE_PLAY &&
        gs->level_current >= 4-1)
    {
        gui->popup = !gui->popup;
        gui->popup_y_vel = 0;
        gui->popup_inventory_y_vel = 0;
        tooltip_reset(&gui->tooltip);
    }

    const f32 speed = 3.0f;

    // For the CONVERTER popup
    if (gui->popup) {
        if (gui->popup_y > 1+round(gs->S*gs->gh)-GUI_POPUP_H) {
            gui->popup_y_vel -= speed;
        } else {
            gui->popup_y_vel = 0;
            gui->popup_y = 1+round(gs->S*gs->gh)-GUI_POPUP_H;
        }
    } else if (gui->popup_y < round(gs->S*gs->gh)) {
        gui->popup_y_vel += speed;
    } else {
        gui->popup_y = (f32) round(gs->S*gs->gh);
        gui->popup_y_vel = 0;
    }

    // For the INVENTORY popup
    if (gui->popup) {
        if (gui->popup_inventory_y < gs->S*GUI_H) {
            gui->popup_inventory_y_vel += speed/2.5f;
        }
        if (gui->popup_inventory_y >= gs->S*GUI_H) {
            gui->popup_inventory_y_vel = 0;
            gui->popup_inventory_y = gs->S*GUI_H;
        }
    } else if (gui->popup_inventory_y > 0) {
        gui->popup_inventory_y_vel -= speed/2.5f;
    } else {
        gui->popup_inventory_y = 0;
        gui->popup_inventory_y_vel = 0;
    }

    if (!gui->popup  && !gs->gui.restart_popup_confirm.active) {
        for (int i = 0; i < TOOL_COUNT; i++) {
            button_tick(gui->tool_buttons[i], &i);
        }
    }

    gui->popup_y += gui->popup_y_vel;
    gui->popup_y = (f32) clamp((int) gui->popup_y, (int) (1 + round(gs->S*gs->gh) - GUI_POPUP_H), gs->window_height);

    gui->popup_inventory_y += gui->popup_inventory_y_vel;
    gui->popup_inventory_y = (f32) clamp((int) gui->popup_inventory_y, 0, GUI_H);
}

// Gives the amount of each cell type there are
// in an array of cells.
static void profile_array(Cell *desired,
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

        if (gs->level_current+1 == 7) {
            sprintf(out[(*count)++], "  %-15s???", name);
        } else if (gs->level_current == 11-1) {
            sprintf(out[(*count)++], "  %-15s???", name);
        } else if (gs->overlay.changes.index < gs->overlay.changes.count-1) {
            sprintf(out[(*count)++], "  %-15s%d??", name, counts[i]);
        } else {
            sprintf(out[(*count)++], "  %-15s%d", name, counts[i]);
        }
    }
}

static bool can_goto_next_level(void) {
    int level = gs->level_current+1;

    if (level == 1 &&
        !compare_cells_to_int(gs->grid, gs->overlay.grid, COMPARE_LEEWAY))
    {
        return false;
    }

    if (level >= 8 &&
        !compare_cells_to_int(gs->grid, gs->overlay.grid, COMPARE_LEEWAY))
    {
        return false;
    }
    return true;
}

static void gui_draw_profile(int target) {
    Assert(target == RENDER_TARGET_MASTER);

    if (gs->converter.active) return;

    Level *level = &gs->levels[gs->level_current];
    int count = 0;

#if 0
    Cell *overlay_grid = PushArray(gs->transient_memory, gs->gw*gs->gh, sizeof(Cell));

    for (int i = 0; i < gs->gw*gs->gh; i++) {
        overlay_grid[i].type = gs->overlay.grid[i];
    }
#endif

    profile_array(level->desired_grid, level->profile_lines, &count);

    int c = 0;

    {
        Render_Text_Data text_data = {0};

        sprintf(text_data.identifier, "Thing");
        text_data.font = gs->fonts.font;
        strcpy(text_data.str, "Required Amounts:");
        text_data.x = Scale(50);
        text_data.y = GUI_H+Scale(50);
        text_data.foreground = (SDL_Color){
            255,
            255,
            0,
        };
        text_data.alpha = 255;
        text_data.background = BLACK;
        text_data.render_type = TEXT_RENDER_LCD;

        RenderDrawText(target, &text_data);

        c = text_data.texture.height;
    }

    for (int i = 0; i < count; i++) {
        Render_Text_Data text_data = {0};

        sprintf(text_data.identifier, "Converter thing %d", i);
        text_data.font = gs->fonts.font;
        strcpy(text_data.str, level->profile_lines[i]);
        text_data.x = Scale(50);
        text_data.y = GUI_H+Scale(50)+c;
        text_data.foreground = WHITE;
        text_data.background = BLACK;
        text_data.alpha = 255;

        RenderDrawText(target, &text_data);

        c += text_data.texture.height;
    }
}

static void gui_draw(int target) {
    Assert(target == RENDER_TARGET_MASTER);

    GUI *gui = &gs->gui;

    // Draw the toolbar buttons.

    RenderColor(0, 0, 0, 0);
    RenderClear(RENDER_TARGET_GUI_TOOLBAR);

    // Tool bar
    if (gs->gui.popup_inventory_y < GUI_H) {
        for (int i = 0; i < TOOL_COUNT; i++) {
            button_draw(RENDER_TARGET_GUI_TOOLBAR, gui->tool_buttons[i]);
        }

        SDL_Rect src = {
            0, 0,
            gs->window_width,
            GUI_H
        };

        SDL_Rect dst = {
            0,
            0,
            gs->window_width,
            GUI_H
        };

        RenderTargetToTarget(target,
                             RENDER_TARGET_GUI_TOOLBAR,
                             &src,
                             &dst);
    }
}

static void gui_popup_draw(int target) {
    Assert(target == RENDER_TARGET_MASTER);

    GUI *gui = &gs->gui;

    SDL_Rect popup = {
        0, (int)(GUI_H + gui->popup_y),
        gs->gw*gs->S, (int)GUI_POPUP_H
    };

    RenderColor(Red(INVENTORY_COLOR),
                Green(INVENTORY_COLOR),
                Blue(INVENTORY_COLOR),
                255);
    RenderFillRect(target, popup);

    RenderColor(Red(CONVERTER_LINE_COLOR),
                Green(CONVERTER_LINE_COLOR),
                Blue(CONVERTER_LINE_COLOR),
                255);

    if (gs->gui.popup_y+GUI_H < gs->window_height) {
        RenderLine(target,
                   0, GUI_H+gui->popup_y-1,
                   gs->window_width,
                   GUI_H+gui->popup_y-1);
    }

    int w, h;
    w = GetTexture(TEXTURE_TAB).width;
    h = GetTexture(TEXTURE_TAB).height;

    SDL_Rect bar = {
        0, popup.y,
        gs->gw*gs->S, Scale(36)
    };

    RenderColorStruct(ColorFromInt(INVENTORY_COLOR2));
    RenderFillRect(target, bar);

    SDL_Rect tab_icon = {
        gs->gw*gs->S - 128, (int)(GUI_H + gui->popup_y) - h,
        w, h
    };

    RenderTextureAlphaMod(&GetTexture(TEXTURE_TAB), 127);

    if (gs->level_current >= 4-1)
        RenderTexture(target, &GetTexture(TEXTURE_TAB), NULL, &tab_icon);

    all_converters_draw(target);
    inventory_draw(target);
    converter_gui_draw();
}

static bool is_cell_stone(int type) {
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
static int get_cell_tier(int type) {
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
