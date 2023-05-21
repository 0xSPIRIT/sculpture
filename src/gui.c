Button *button_allocate(enum Button_Type type, SDL_Texture *texture, const char *tooltip_text, void (*on_pressed)(void*)) {
    Button *b = PushSize(gs->persistent_memory, sizeof(Button));
    b->type = type;
    b->texture = texture;
    b->disabled = false;
    
    b->index=-1;
    
    SDL_QueryTexture(texture, NULL, NULL, &b->w, &b->h);
    
    strcpy(b->tooltip_text, tooltip_text);
    b->on_pressed = on_pressed;
    return b;
}

void tool_button_set_disabled(int level) {
    Button **tools = gs->gui.tool_buttons;
    
    //tools[TOOL_BLOCKER]->disabled = true;
    
    if (compare_cells_to_int(gs->grid, gs->overlay.grid, COMPARE_LEEWAY)) {
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
            tools[TOOL_DELETER]->disabled = true;
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

void click_gui_tool_button(void *type_ptr) {
    int type = *(int*)type_ptr;
    
    GUI *gui = &gs->gui;
    
    if (gui->popup) return;
    
    int p_tool = gs->current_tool;
    
    gs->current_tool = type;
    
    gui->tool_buttons[type]->highlighted = false;
    
    switch (gs->current_tool) {
        case TOOL_CHISEL_SMALL: {
            gs->chisel = &gs->chisel_small;
            //gs->chisel_hammer.normal_dist = gs->chisel_hammer.dist = (f32) (gs->chisel->w+2);
            break;
        }
        case TOOL_CHISEL_MEDIUM: {
            gs->chisel = &gs->chisel_medium;
            //gs->chisel_hammer.normal_dist = gs->chisel_hammer.dist = (f32) (gs->chisel->w+4);
            break;
        }
        case TOOL_CHISEL_LARGE: {
            gs->current_tool = TOOL_CHISEL_LARGE;
            gs->chisel = &gs->chisel_large;
            //gs->chisel_hammer.normal_dist = gs->chisel_hammer.dist = (f32) (gs->chisel->w+4);
            break;
        }
        case TOOL_OVERLAY: {
            gs->overlay.show = !gs->overlay.show;
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

void button_tick(Button *b, void *data) {
    Input *input = &gs->input;
    GUI *gui = &gs->gui;
    
    int gui_input_mx = input->real_mx;
    int gui_input_my = input->real_my;
    
    // This is a hack so that function pointers won't stop working
    // upon reloading the DLL... Honestly why don't we just do it
    // this way normally? Seems to work great.
    switch (b->type) {
        case BUTTON_TYPE_CONVERTER:     b->on_pressed = converter_begin_converting; break;
        case BUTTON_TYPE_TOOL_BAR:      b->on_pressed = click_gui_tool_button;      break;
        case BUTTON_TYPE_TUTORIAL:      b->on_pressed = tutorial_rect_close;        break;
        case BUTTON_TYPE_POPUP_CONFIRM: b->on_pressed = popup_confirm_confirm;      break;
        case BUTTON_TYPE_POPUP_CANCEL:  b->on_pressed = popup_confirm_cancel;       break;
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
        }
    } else if (b->just_had_tooltip) {
        b->just_had_tooltip = false;
        tooltip_reset(&gui->tooltip);
    }
}

void button_draw(Button *b) {
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
        SDL_SetTextureColorMod(b->texture, 90, 90, 90);
    } else if (b->active) {
        SDL_SetTextureColorMod(b->texture, 200, 200, 200);
    } else if (b->highlighted) {
        f64 c = (sin(SDL_GetTicks()/100.0)+1)/2.0;
        c *= 128;
        SDL_SetTextureColorMod(b->texture, 255-c, 255, 255-c);
    } else if (gui_input_mx >= b->x && gui_input_mx < b->x+b->w &&
               gui_input_my >= b->y && gui_input_my < b->y+b->h) {
        SDL_SetTextureColorMod(b->texture, 230, 230, 230);
    } else {
        SDL_SetTextureColorMod(b->texture, 255, 255, 255);
    }
    
    SDL_RenderCopy(gs->renderer, b->texture, NULL, &dst);
}

void gui_message_stack_push(const char *str) {
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

void gui_message_stack_tick_and_draw(void) {
    GUI *gui = &gs->gui;
    
    for (int i = 0; i < gui->message_count; i++) {
        Message *msg = &gui->message_stack[i];
        
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
    GUI *gui = &gs->gui;
    
    gui->popup_y = (f32) (gs->gh*gs->S);
    gui->popup_y_vel = 0;
    gui->popup = 0;
    gui->popup_texture = Texture(TEXTURE_POPUP);
    
    gs->gui.popup_confirm = popup_confirm_init();
    
    tooltip_reset(&gui->tooltip);
    
    int cum = 0;
    
    for (int i = 0; i < TOOL_COUNT; i++) {
        char name[128] = {0};
        get_name_from_tool(i, name);
        
        if (gui->tool_buttons[i] == NULL) {
            gui->tool_buttons[i] = button_allocate(BUTTON_TYPE_TOOL_BAR,
                                                   Texture(TEXTURE_TOOL_BUTTONS + i),
                                                   name,
                                                   click_gui_tool_button);
            gui->tool_buttons[i]->w = gui->tool_buttons[i]->h = GUI_H;
            SDL_SetTextureScaleMode(gui->tool_buttons[i]->texture, 1);
            if (gs->tool_previews[i].length)
                gui->tool_buttons[i]->preview = &gs->tool_previews[i];
        }
        
        gui->tool_buttons[i]->x = cum;
        gui->tool_buttons[i]->y = 0;
        gui->tool_buttons[i]->index = i;
        gui->tool_buttons[i]->active = i == gs->current_tool;
        
        cum += gui->tool_buttons[i]->w;
    }
    
#if 0
    SDL_SetTextureScaleMode(RenderTarget(RENDER_TARGET_GUI_TOOLBAR),
                            2);
#endif
    
    //overlay_interface_init();
}

void gui_tick(void) {
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
    
    if (!gui->popup) {
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
void profile_array(Cell *desired,
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

void gui_draw_profile() {
    Level *level = &gs->levels[gs->level_current];
    int count = 0;
    
#if 0
    Cell *overlay_grid = PushArray(gs->transient_memory, gs->gw*gs->gh, sizeof(Cell));
    
    for (int i = 0; i < gs->gw*gs->gh; i++) {
        overlay_grid[i].type = gs->overlay.grid[i];
    }
#endif
    
    profile_array(level->desired_grid, level->profile_lines, &count);
    
    int ah = 0;
    draw_text_indexed(TEXT_CONVERTER_REQUIRED_START,
                      gs->fonts.font,
                      "Required Amounts:",
                      (SDL_Color){
                          255,
                          255,
                          0,
                          255
                      },
                      BLACK,
                      255,
                      false,
                      false,
                      50,
                      GUI_H+50,
                      NULL,
                      &ah,
                      false);
    
    int c = ah;
    for (int i = 0; i < count; i++) {
        int h;
        draw_text_indexed(TEXT_CONVERTER_REQUIRED_START+1+i,
                          gs->fonts.font,
                          level->profile_lines[i],
                          WHITE,
                          BLACK,
                          255,
                          false,
                          false,
                          50,
                          GUI_H+50+c,
                          NULL,
                          &h,
                          false);
        c += h;
    }
}

void gui_draw(void) {
    GUI *gui = &gs->gui;
    
    // Draw the toolbar buttons.
    SDL_Texture *old = SDL_GetRenderTarget(gs->renderer);
    
    SDL_SetTextureBlendMode(RenderTarget(RENDER_TARGET_GUI_TOOLBAR), SDL_BLENDMODE_BLEND);
    
    Assert(RenderTarget(RENDER_TARGET_GUI_TOOLBAR));
    SDL_SetRenderTarget(gs->renderer, RenderTarget(RENDER_TARGET_GUI_TOOLBAR));
    
    SDL_SetRenderDrawColor(gs->renderer, 0, 0, 0, 0);
    SDL_RenderClear(gs->renderer);
    
    // Tool bar
    if (gs->gui.popup_inventory_y < GUI_H) {
        for (int i = 0; i < TOOL_COUNT; i++) {
            button_draw(gui->tool_buttons[i]);
        }
        
        SDL_Rect dst = {
            0, 0,
            gs->window_width, GUI_H
        };
        
        SDL_SetRenderTarget(gs->renderer, RenderTarget(RENDER_TARGET_MASTER));
        SDL_RenderCopy(gs->renderer, RenderTarget(RENDER_TARGET_GUI_TOOLBAR), &dst, &dst);
    }
    
    popup_confirm_tick_and_draw(&gs->gui.popup_confirm);
    
    SDL_SetRenderTarget(gs->renderer, old);
}


//~ Popup Confirmation

void popup_confirm_confirm(void* ptr) {
    (void)ptr;
    Popup_Confirm *c = &gs->gui.popup_confirm;
    Level *level = &gs->levels[gs->level_current];
    
    c->active = false;
    
    set_fade(FADE_LEVEL_FINISH, 0, 255);
    
    level->off = false;
    level->desired_alpha = 0;
    level_set_state(gs->level_current, LEVEL_STATE_PLAY);
}

void popup_confirm_cancel(void* ptr) {
    (void)ptr;
    
    Popup_Confirm *c = &gs->gui.popup_confirm;
    Level *level = &gs->levels[gs->level_current];
    
    level->off = false;
    level->desired_alpha = 0;
    level_set_state(gs->level_current, LEVEL_STATE_PLAY);
    
    c->active = false;
}

Popup_Confirm popup_confirm_init() {
    Popup_Confirm result = {0};
    
    result.active = false;
    
    result.a = button_allocate(BUTTON_TYPE_POPUP_CONFIRM,
                               Texture(TEXTURE_CONFIRM_BUTTON),
                               "",
                               popup_confirm_confirm);
    result.b = button_allocate(BUTTON_TYPE_POPUP_CANCEL,
                               Texture(TEXTURE_CANCEL_BUTTON),
                               "",
                               popup_confirm_cancel);
    
    return result;
}

void popup_confirm_tick_and_draw(Popup_Confirm *popup) {
    if (wait_for_fade(FADE_LEVEL_FINISH)) {
        reset_fade();
        goto_level(++gs->level_current);
        return;
    }
    
    if (!popup->active) return;
    
    popup->r = (SDL_Rect){
        gs->window_width/6,
        gs->window_height/3,
        2*gs->window_width/3,
        gs->window_height/4
    };
    SDL_SetRenderDrawColor(gs->renderer, 32, 32, 32, 255);
    SDL_RenderFillRect(gs->renderer, &popup->r);
    
    popup->a->x = 1*gs->window_width/5 + popup->b->w*1;
    popup->a->y = popup->r.y + popup->r.h - Scale(64);
    
    popup->b->x = 4*gs->window_width/5 - popup->a->w*2;
    popup->b->y = popup->r.y + popup->r.h - Scale(64);
    
    button_tick(popup->a, NULL);
    button_tick(popup->b, NULL);
    
    button_draw(popup->a);
    button_draw(popup->b);
    
    SDL_Color col = (SDL_Color){175, 175, 175, 255};
    SDL_Color bg = (SDL_Color){32,32,32,255};
    
    int w;
    
    char *text = "Are you satisfied with this result?";
    
    TTF_SizeText(gs->fonts.font_times, text, &w, NULL);
    
    draw_text_indexed(TEXT_CONFIRM,
                      gs->fonts.font_times,
                      "Confirmation",
                      col,
                      bg,
                      255,
                      false,
                      false,
                      popup->r.x + Scale(16),
                      popup->r.y + Scale(10),
                      NULL,
                      NULL,
                      false);
    
    col = (SDL_Color){255, 255, 255, 255};    
    draw_text_indexed(TEXT_CONFIRM,
                      gs->fonts.font_times,
                      text,
                      col,
                      bg,
                      255,
                      false,
                      false,
                      popup->r.x + popup->r.w/2 - w/2,
                      popup->r.y + Scale(70),
                      NULL,
                      NULL,
                      false);
    
    
    if (gs->level_current+1 >= 8 && !compare_cells_to_int(gs->grid, gs->overlay.grid, COMPARE_LEEWAY)) {
        char comment[128]={0};
        
        strcpy(comment, "I cannot settle for this.");
        
        int h;
        
        TTF_SizeText(gs->fonts.font, comment, &w, &h);
        
        draw_text_indexed(TEXT_NOT_GOOD_ENOUGH,
                          gs->fonts.font,
                          comment,
                          (SDL_Color){180, 0, 0, 255},
                          BLACK,
                          255,
                          0, 0,
                          popup->r.x + popup->r.w/2 - w/2,
                          popup->r.y + popup->r.h - 1.5*h,
                          NULL, NULL,
                          false);
    }
    
}

void gui_popup_draw(void) {
    GUI *gui = &gs->gui;
    
    SDL_Rect popup = {
        0, (int)(GUI_H + gui->popup_y),
        gs->gw*gs->S, (int)GUI_POPUP_H
    };
    
    SDL_SetRenderDrawColor(gs->renderer,
                           Red(INVENTORY_COLOR),
                           Green(INVENTORY_COLOR),
                           Blue(INVENTORY_COLOR),
                           255);
    SDL_RenderFillRect(gs->renderer, &popup);
    
    SDL_SetRenderDrawColor(gs->renderer, 
                           Red(CONVERTER_LINE_COLOR),
                           Green(CONVERTER_LINE_COLOR),
                           Blue(CONVERTER_LINE_COLOR),
                           255);
    if (gs->gui.popup_y+GUI_H < gs->window_height) {
        SDL_RenderDrawLine(gs->renderer,
                           0, GUI_H+gui->popup_y-1,
                           gs->window_width, GUI_H+gui->popup_y-1);
    }
    
    int w, h;
    SDL_QueryTexture(Texture(TEXTURE_TAB), NULL, NULL, &w, &h);
    
    SDL_Rect bar = {
        0, popup.y,
        gs->gw*gs->S, Scale(36)
    };
    
    SDL_SetRenderDrawColor(gs->renderer,
                           Red(INVENTORY_COLOR2),
                           Green(INVENTORY_COLOR2), 
                           Blue(INVENTORY_COLOR2), 255);
    SDL_RenderFillRect(gs->renderer, &bar);
    
    SDL_Rect tab_icon = {
        gs->gw*gs->S - 128, (int)(GUI_H + gui->popup_y) - h,
        w, h
    };
    
    SDL_SetTextureAlphaMod(Texture(TEXTURE_TAB), 127);
    
    if (gs->level_current >= 4-1)
        SDL_RenderCopy(gs->renderer, Texture(TEXTURE_TAB), NULL, &tab_icon);
    
    all_converters_draw();
    inventory_draw();
    converter_gui_draw();
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