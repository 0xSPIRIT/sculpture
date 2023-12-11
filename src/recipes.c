static int get_item_from_any(int type, int timer, int override_index) {
    Assert(type < 0);
    timer /= 2;

    const int step = 30; // The interval between switches.

    if (override_index != -1) {
        timer = override_index * step;
    }

    Log("Type: %d\n", type);

    switch(type) {
        case CELL_ANY_STONE: {
            int t = timer % (step*7);

            if (t < step)    return CELL_STONE;
            if (t < step*2)  return CELL_MARBLE;
            if (t < step*3)  return CELL_SANDSTONE;
            if (t < step*4)  return CELL_QUARTZ;
            if (t < step*5)  return CELL_GRANITE;
            if (t < step*6)  return CELL_BASALT;
            if (t < step*7)  return CELL_DIAMOND;

        } break;
        case CELL_ANY_COAL: {
            int t = timer % (step*2);

            if (t < step)    return CELL_UNREFINED_COAL;
            if (t < step*2)  return CELL_REFINED_COAL;

        } break;
        case CELL_ANY_CONVERT_TO_COAL: {
            int t = timer % (step*5);

            if (t < step)    return CELL_MARBLE;
            if (t < step*2)  return CELL_STONE;
            if (t < step*3)  return CELL_SANDSTONE;
            if (t < step*4)  return CELL_DIRT;
            if (t < step*5)  return CELL_SAND;

        } break;
        case CELL_ANY_FUEL: {
            int t = timer % (step*3);

            if (t < step)    return CELL_UNREFINED_COAL;
            if (t < step*2)  return CELL_REFINED_COAL;
            if (t < step*3)  return CELL_LAVA;
        } break;
        case CELL_ANY_STEAM_OR_ICE: {
            int t = timer % (step*2);

            if (t < step)    return CELL_STEAM;
            if (t < step*2)  return CELL_ICE;
        } break;
        case CELL_ANY_NONE_OR_UNREFINED_COAL: {
            int t = timer % (step*2);

            if (t < step)    return CELL_NONE;
            if (t < step*2)  return CELL_UNREFINED_COAL;
        } break;
        case CELL_ANY_WATER_OR_ICE: {
            int t = timer % (step*2);

            if (t < step)    return CELL_WATER;
            if (t < step*2)  return CELL_ICE;
        } break;
        default: {
            Assert(0); // Unknown any type.
        } break;
    }

    return 0;
}

static void converter_gui_init(void) {
    Recipes* c = &gs->recipes;

    const Recipe_Item conversions[] = {
        // Converter,  Fuel, Input 1, Input 2, Output, Alternate button
        {CONVERTER_FUEL,     0,                   CELL_ANY_CONVERT_TO_COAL, 0,                 CELL_UNREFINED_COAL, false},
        {CONVERTER_FUEL,     0,                   CELL_UNREFINED_COAL,      CELL_GLASS,        CELL_REFINED_COAL,   false},
        {CONVERTER_FUEL,     0,                   CELL_ANY_STONE,           CELL_REFINED_COAL, CELL_LAVA,           false},

        {CONVERTER_MATERIAL, CELL_ANY_FUEL,       CELL_STONE,            0,           CELL_MARBLE,    false},
        {CONVERTER_MATERIAL, CELL_UNREFINED_COAL, CELL_STONE,            CELL_SAND,   CELL_SANDSTONE, false},
        {CONVERTER_MATERIAL, CELL_REFINED_COAL,   CELL_SANDSTONE,        CELL_MARBLE, CELL_QUARTZ,    false},
        {CONVERTER_MATERIAL, CELL_UNREFINED_COAL, CELL_SAND,             0,           CELL_GLASS,     false},

        {CONVERTER_MATERIAL, CELL_ANY_NONE_OR_UNREFINED_COAL, CELL_ANY_STEAM_OR_ICE, 0, CELL_WATER, true},
        {CONVERTER_MATERIAL, 0,                               CELL_WATER,            0, CELL_ICE,   false},

        {CONVERTER_MATERIAL, CELL_ANY_COAL,       CELL_DIRT,             0,         CELL_STONE,     false},
        {CONVERTER_MATERIAL, CELL_ANY_COAL,       CELL_ANY_WATER_OR_ICE, 0,         CELL_STEAM,     true},

        {CONVERTER_MATERIAL, CELL_LAVA, CELL_ANY_COAL, 0,            CELL_BASALT,  false},
        {CONVERTER_MATERIAL, CELL_LAVA, CELL_QUARTZ,   CELL_MARBLE,  CELL_GRANITE, false},
        {CONVERTER_MATERIAL, CELL_LAVA, CELL_BASALT,   CELL_GRANITE, CELL_DIAMOND, false},
    };

    Recipe_Item *ptr = c->conversions;
    if (!ptr) {
        ptr = PushSize(gs->persistent_memory, sizeof(conversions));
    }
    memset(c, 0, sizeof(Recipes));

    c->conversions = ptr;

    memcpy(c->conversions, conversions, sizeof(conversions));
    c->conversions_count = ArrayCount(conversions);

    for (int i = 0; i < c->conversions_count; i++) {
        if (conversions[i].alternate_button)
            c->override_indices[i] = 1;
    }
}

static bool converter_gui_item_draw(int target, int item, int override_index, int x, int y, int w, int h) {
    bool mouse_in_rect = false;

    SDL_Rect r = { x, y, w, h };

    if (r.x+r.w < 0 || r.y+r.h < 0 || r.x >= gs->game_width || r.y >= gs->game_height) return false;

    if (item < 0) {
        item = get_item_from_any(item, gs->recipes.timer, override_index-1);
    }

    SDL_Point mouse = {gs->input.real_mx, gs->input.real_my};
    if (is_point_in_rect(mouse, r) && item != 0) {
        tooltip_reset(&gs->gui.tooltip);
        tooltip_set_position_to_cursor(&gs->gui.tooltip, item);
        memset(gs->gui.tooltip.str, 0, MAX_TOOLTIP_LEN*MAX_TOOLTIP_LINE_LEN);

        get_name_from_type(item, gs->gui.tooltip.str[0]);

        if (item != CELL_DIRT && item != CELL_SAND) {
            strcpy(gs->gui.tooltip.str[1], "Click to go to recipe");
        }

        mouse_in_rect = true;

        if (gs->input.mouse_pressed[SDL_BUTTON_LEFT]) {
            gs->recipes.definition = item;
            gs->recipes.definition_alpha = 255;
        }
    }

    RenderColor(0, 0, 0, 255);
    RenderFillRect(target, r);
    RenderColor(128, 128, 128, 255);
    RenderDrawRect(target, r);

    Texture *texture = &GetTexture(TEXTURE_ITEMS+item);

    RenderTextureColorMod(texture, 255, 255, 255);
    RenderTexture(target, texture, null, &r);

    return mouse_in_rect;
}

static void converter_draw_arrow(int target, int xx, int yy) {
    int head_size = Scale(15);
    int length = Scale(40);
    RenderArrow(target, (SDL_Point){xx, yy}, (SDL_Point){xx, yy+length}, head_size);
}

static void recipes_draw_text(int target, const char *str, int item_x, int item_y, int item_w) {
    char ident[32] = {0};
    strncpy(ident, str, 32);

    int w, h;
    TTF_SizeText(gs->fonts.font->handle, str, &w, &h);

    int new_x = item_x + item_w/2 - w/2;
    int new_y = item_y-h;
    if (new_x+w < 0 || new_y+h < 0 || new_x >= gs->game_width || new_y >= gs->game_height) return;

    RenderTextQuick(target,
                    ident,
                    gs->fonts.font,
                    str,
                    WHITE,
                    new_x,
                    new_y,
                    null,
                    null,
                    false);
}

// Horrible code. Avert your eyes.
// I think this is the worst code I've ever written.
// Why are these functions and not constants?

static inline int _recipes_get_jump_dy(void) {
    return Scale(250);
}

static inline int _recipes_get_fuel_dx(void) {
    return Scale(16+768-310);
}

static inline int _recipes_get_material_dx(void) {
    return Scale(66);
}

static inline int _recipes_get_dy(void) {
    return gs->recipes.y + GUI_H*3; // These values are already scaled
}

static inline int _recipes_get_input_dx(void) {
    return Scale(128);
}

static inline int _recipes_get_fuel_dx_material(void) {
    return Scale(216);
}

static inline int _recipes_get_fuel_dy(void) {
    return Scale(56);
}

static inline int _recipes_get_output_offset(void) {
    return Scale(96);
}

// Beginning to regret using C....!
typedef struct GuiConversionPair {
    int output;
    SDL_Rect rectangle;
} GuiConversionPair;

// Add the rectangle to the array.
static void recipe_pair_add(GuiConversionPair *pairs, int *pair_count, int output, int x, int y) {
    Assert(pairs);
    Assert(pair_count);
    pairs[(*pair_count)++] = (GuiConversionPair){ output, (SDL_Rect){x, y, Scale(238), Scale(200)} };
}

static SDL_Rect recipes_get_rect(GuiConversionPair *pairs, int pair_count, int output) {
    for (int i = 0; i < pair_count; i++) {
        if (pairs[i].output == output) {
            return pairs[i].rectangle;
        }
    }

    return (SDL_Rect){-1, -1, -1, -1};
}

static void recipes_draw_scroll_bar(int target) {
    Recipes*c = &gs->recipes;

    int w = Scale(16);
    int h = gs->game_height-GUI_H;
    int bar_height = Scale(64);

    int logical_height = gs->game_height-GUI_H-bar_height;


    SDL_Point mouse = {gs->input.real_mx, gs->input.real_my};


    SDL_Rect scroll_bar_full = {
        gs->game_width - w,
        GUI_H,
        w,
        h
    };

    SDL_Rect scroll_bar = {
        gs->game_width - w,
        GUI_H + logical_height * -c->y/c->max_height,
        w,
        bar_height
    };
    if (is_point_in_rect(mouse, scroll_bar) && gs->input.mouse_pressed[SDL_BUTTON_LEFT]) {
        c->holding_scroll_bar = true;
    }

    if (!(gs->input.mouse & SDL_BUTTON_LEFT)) {
        c->holding_scroll_bar = false;
    }

    if (c->holding_scroll_bar) {
        f64 mouse_dy = gs->input.real_my - gs->input.real_pmy;

        // Set the view y position based on the scroll bar.
        c->y -= mouse_dy/logical_height * c->max_height;

        c->y = clamp(c->y, -c->max_height, 0);
        c->y_to = c->y;
    }

    RenderColor(32, 32, 32, 255);
    RenderFillRect(target, scroll_bar_full);
    RenderColor(255, 255, 255, 255);
    RenderFillRect(target, scroll_bar);
}

static bool can_recipes_be_active(void) {
    return gs->level_current+1 >= 4;
}

static void gui_converter_toggle(void) {
    gs->recipes.active = !gs->recipes.active;
    tooltip_reset(&gs->gui.tooltip);
}

static void recipes_draw_button(int target) {
    Button *b = &gs->recipes.button;

    if (gs->gui.popup_inventory_y <= 0) return;
    if (!can_recipes_be_active()) return;

    b->index = -1;
    b->w = ITEM_SIZE;
    b->h = ITEM_SIZE;
    b->x = gs->game_width - b->w*2;
    b->y = gs->gui.popup_inventory_y - b->h/2 - GUI_H/2;
    b->texture = &GetTexture(TEXTURE_RECIPE_BOOK_BUTTON);
    strcpy(b->tooltip_text, "Recipe Book");
    b->active = true;
    b->on_pressed = null;

    bool pressed = button_tick(b, null);
    button_draw(target, b);

    if (pressed) {
        gui_converter_toggle();
    }
}

static void recipe_draw_note(int target) {
    int dy = Scale(45) + gs->recipes.y;

    RenderText(target,
               &(Render_Text_Data){
                   .identifier = "2matc",
                   .font = gs->fonts.font_times,
                   .str = "1 Fuel -> 8 Output",
                   .x = gs->game_width/4,
                   .y = GUI_H + dy + Scale(64),
                   .foreground = WHITE,
                   .alignment = ALIGNMENT_CENTER,
                   .render_type = TEXT_RENDER_BLENDED,
               });
}

static void recipes_draw(int final_target) {
    int target = RENDER_TARGET_CONVERSION_PANEL;

    Recipes *c = &gs->recipes;
    gs->recipes.timer++;

    if (!gs->recipes.active) return;

    RenderMaybeSwitchToTarget(target);

    RenderColor(0,0,0,175);
    RenderClear(target);

    if (gs->input.keys[SDL_SCANCODE_ESCAPE]) {
        gui_converter_toggle();
        gs->input.keys[SDL_SCANCODE_ESCAPE] = 0; // eat the input
        return;
    }

    gs->recipes.max_height = Scale(2400);

    int dx = _recipes_get_fuel_dx();
    int dy = _recipes_get_dy();

    int cum = 0;

    int size = Scale(48);

    gs->recipes.y = lerp64(gs->recipes.y, gs->recipes.y_to, 0.5);

    bool mouse_in_none = true;

    Font *title_font = gs->fonts.font_times;
    int yoff = Scale(45)+c->y;

    int pad = Scale(30);

    GuiConversionPair pairs[9999] = {0};
    int pair_count = 0;

    // Fuel recipes
    RenderTextQuick(target,
                    "fuelc",
                    title_font,
                    "Fuel Conversions",
                    WHITE,
                    dx,
                    GUI_H+yoff,
                    null,
                    null,
                    false);

    for (int i = 0; i < 3; i++) {
        Recipe_Item item = c->conversions[i];

        Assert(item.converter == CONVERTER_FUEL);

        {
            int x = dx;
            int y = dy + cum;

            recipe_pair_add(pairs, &pair_count, item.output, x - pad, y-gs->recipes.y - pad);

            recipes_draw_text(target, "Input 1", x, y, size);
            bool mouse_in = converter_gui_item_draw(target,
                                                    item.input1,
                                                    c->override_indices[i],
                                                    x,
                                                    y,
                                                    size,
                                                    size);
            if (mouse_in) mouse_in_none = false;
        }

        {
            int x = dx + _recipes_get_input_dx();
            int y = dy + cum;

            recipes_draw_text(target, "Input 2", x, y, size);
            bool mouse_in = converter_gui_item_draw(target,
                                                    item.input2,
                                                    c->override_indices[i],
                                                    x,
                                                    y,
                                                    size,
                                                    size);
            if (mouse_in) mouse_in_none = false;
        }

        {
            int x = ((dx)+(dx+_recipes_get_input_dx()))/2;
            int y = dy + _recipes_get_output_offset() + cum;

            { // Arrow
                RenderColor(255,255,255,255);
                int xx = dx + size/2 + dx + _recipes_get_input_dx() + size/2;
                xx /= 2;

                int yy = dy+cum+20;

                converter_draw_arrow(target, xx, yy);
            }

            recipes_draw_text(target, "Output", x, y, size);
            bool mouse_in = converter_gui_item_draw(target,
                                                    item.output,
                                                    c->override_indices[i],
                                                    x,
                                                    y,
                                                    size,
                                                    size);
            if (mouse_in) mouse_in_none = false;
        }

        cum += _recipes_get_jump_dy();
    }

    // Material recipes
    dx = _recipes_get_material_dx();
    cum = 0;

    RenderTextQuick(target,
                    "matc",
                    title_font,
                    "Material Conversions",
                    WHITE,
                    dx,
                    GUI_H+yoff,
                    null,
                    null,
                    false);

    //recipe_draw_note(target);

    for (int i = 3; i < c->conversions_count; i++) {
        Recipe_Item item = c->conversions[i];

        Assert(item.converter == CONVERTER_MATERIAL);

        {
            int x = dx + _recipes_get_fuel_dx_material();
            int y = dy + cum + _recipes_get_fuel_dy();

            recipes_draw_text(target, "Fuel", x, y, size);
            bool mouse_in = converter_gui_item_draw(target,
                                                    item.fuel,
                                                    c->override_indices[i],
                                                    x,
                                                    y,
                                                    size,
                                                    size);
            if (mouse_in) mouse_in_none = false;
        }

        {
            int x = dx;
            int y = dy + cum;

            recipe_pair_add(pairs, &pair_count, item.output, x - pad, y - gs->recipes.y - pad);

            recipes_draw_text(target, "Input 1", x, y, size);
            bool mouse_in = converter_gui_item_draw(target,
                                                    item.input1,
                                                    c->override_indices[i],
                                                    x,
                                                    y,
                                                    size,
                                                    size);
            if (mouse_in) mouse_in_none = false;
        }

        {
            int x = dx + _recipes_get_input_dx();
            int y = dy + cum;

            recipes_draw_text(target, "Input 2", x, y, size);
            bool mouse_in = converter_gui_item_draw(target,
                                                    item.input2,
                                                    c->override_indices[i],
                                                    x,
                                                    y,
                                                    size,
                                                    size);
            if (mouse_in) mouse_in_none = false;
        }

        {
            int x = ((dx)+(dx+_recipes_get_input_dx()))/2;
            int y = dy +  _recipes_get_output_offset() + cum;

            { // Arrow
                RenderColor(255,255,255,255);
                int xx = dx + size/2 + dx + _recipes_get_input_dx() + size/2;
                xx /= 2;

                int yy = dy+cum+20;

                converter_draw_arrow(target, xx, yy);
            }

            recipes_draw_text(target, "Output", x, y, size);
            bool mouse_in = converter_gui_item_draw(target,
                                                    item.output,
                                                    c->override_indices[i],
                                                    x,
                                                    y,
                                                    size,
                                                    size);
            if (mouse_in) mouse_in_none = false;
        }

        if (item.alternate_button) {
            Button b = {0};
            b.texture = &GetTexture(TEXTURE_ALTERNATE_BUTTON);
            b.x = dx + _recipes_get_fuel_dx_material();
            b.y = dy + cum + _recipes_get_fuel_dy() - Scale(80);
            b.w = Scale(48);
            b.h = b.w;
            b.on_pressed = null;
            b.index = -1;
            strcpy(b.tooltip_text, "Alternate recipe");

            SDL_Point mouse = {gs->input.real_mx, gs->input.real_my};
            if (is_point_in_rect(mouse, (SDL_Rect){b.x,b.y,b.w,b.h})) {
                mouse_in_none = false;
            }

            bool clicked = button_tick(&b, null);
            button_draw(target, &b);

            if (clicked) {
                if (c->override_indices[i] != 2) {
                    c->override_indices[i] = 2;
                } else {
                    c->override_indices[i] = 1;
                }
            }
        }

        cum += _recipes_get_jump_dy();
    }

    RenderColor(255, 255, 255, 255);
    RenderLine(target, gs->game_width/2, GUI_H, gs->game_width/2, gs->game_height);

    if (mouse_in_none) {
        tooltip_reset(&gs->gui.tooltip);
    }

    if (c->definition_alpha) {
        int type = c->definition;

        SDL_Rect r = recipes_get_rect(pairs, pair_count, type);

        if (r.x == -1 && c->definition_alpha) c->definition_alpha = 0;

        if (r.x != -1 && c->definition_alpha == 255) { // First frame clicked?
            // Set up the scroll position
            // Find the average of all the rects' y positions.

            c->y_to = -r.y + (gs->game_height-GUI_H)/2;
            c->y_to = clamp(c->y_to, -c->max_height, 0);
        }

        RenderColor(255, 0, 0, c->definition_alpha);
        r.y += c->y;
        RenderDrawRect(target, r);
        c->definition_alpha--;

        if (c->definition_alpha <= 0) {
            c->definition_alpha = 0;
            c->definition = 0;
        }
    }


    recipes_draw_scroll_bar(target);

    SDL_Rect src = {
        0, GUI_H,
        gs->game_width,
        gs->game_height-GUI_H
    };

    SDL_Rect dst = {
        0,
        GUI_H,
        gs->game_width,
        gs->game_height-GUI_H
    };

    RenderTargetToTarget(final_target,
                         target,
                         &src,
                         &dst);
}
