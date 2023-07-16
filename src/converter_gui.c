// TODO:
//  - the get_rects function doesn't work when you click on something
//  - redo all scroll bar code. It misses the diamond at the bottom, and is generally not very nice!
//  - It's slow! Maybe cache the entire render target? You should profile it first.
//    frame time is 10-15ms, which is not ideal, since we're usually looking at
//    1-6ms.
//  - Seperate the two sides by a line.

int get_item_from_any(int type, int timer) {
    Assert(type < 0);
    timer /= 2;
    
    if (type == CELL_ANY_STONE) {
        int t = timer % 140;
        if (t < 20) {
            return CELL_STONE;
        } else if (t < 40) {
            return CELL_MARBLE;
        } else if (t < 60) {
            return CELL_SANDSTONE;
        } else if (t < 80) {
            return CELL_QUARTZ;
        } else if (t < 100) {
            return CELL_GRANITE;
        } else if (t < 120) {
            return CELL_BASALT;
        } else if (t < 140) {
            return CELL_DIAMOND;
        }
    } else if (type == CELL_ANY_COAL) {
        int t = timer % 40;
        if (t < 20) {
            return CELL_UNREFINED_COAL;
        } else if (t < 40) {
            return CELL_REFINED_COAL;
        }
    } else {
        Assert(0);
    }
    
    return 0;
}

bool converter_gui_item_draw(int target, Cell_Type item, int x, int y, int w, int h) {
    bool mouse_in_rect = false;
    
    SDL_Rect r = {
        Scale(x),
        Scale(y),
        Scale(w),
        Scale(h)
    };
    
    if (r.x+r.w < 0 || r.y+r.h < 0 || r.x >= gs->window_width || r.y >= gs->window_height) return false;
    
    if (item < 0) {
        item = get_item_from_any(item, gs->conversions.timer);
    }
    
    SDL_Point mouse = {gs->input.real_mx, gs->input.real_my};
    if (is_point_in_rect(mouse, r) && item != 0) {
        tooltip_reset(&gs->gui.tooltip);
        tooltip_set_position_to_cursor(&gs->gui.tooltip, item);
        memset(gs->gui.tooltip.str, 0, MAX_TOOLTIP_LEN*MAX_TOOLTIP_LINE_LEN);
        
        get_name_from_type(item, gs->gui.tooltip.str[0]);
        
        strcpy(gs->gui.tooltip.str[1], "Click to go to definition");
        mouse_in_rect = true;
        
        if (gs->input.mouse_pressed[SDL_BUTTON_LEFT]) {
            gs->conversions.definition = item;
            gs->conversions.definition_alpha = 255;
        }
    }
    
    RenderColor(32, 32, 32, 255);
    RenderFillRect(target, r);
    RenderColor(128, 128, 128, 255);
    RenderDrawRect(target, r);
    
    RenderTexture(target,
                  &GetTexture(TEXTURE_ITEMS+item),
                  null,
                  &r);
    
    return mouse_in_rect;
}

void converter_draw_arrow(int target, int xx, int yy) {
    int head_size = 15;
    int length = 40;
    RenderArrow(target, (SDL_Point){xx, yy}, (SDL_Point){xx, yy+Scale(length)}, Scale(head_size));
}

void converter_gui_draw_text(int target,
                             const char *str,
                             int item_x,
                             int item_y,
                             int item_w,
                             int idx)
{
    char ident[32] = {0};
    sprintf(ident, "%d", idx);
    
    int w, h;
    TTF_SizeText(gs->fonts.font->handle, str, &w, &h);
    
    int new_x = Scale(item_x) + item_w/2 - w/2;
    int new_y = Scale(item_y)-h;
    if (new_x+w < 0 || new_y+h < 0 || new_x >= gs->window_width || new_y >= gs->window_height) return;
    
    RenderTextQuick(target,
                    ident,
                    gs->fonts.font,
                    str,
                    WHITE,
                    255,
                    new_x,
                    new_y,
                    null,
                    null,
                    false);
}

// All of these values must be scaled upon use.

static inline int _gui_conversions_get_jump_dy(void) {
    return 250;
}

static inline int _gui_conversions_get_fuel_dx(void) {
    return 16+768-300;
}

static inline int _gui_conversions_get_material_dx(void) {
    return 66;
}

static inline int _gui_conversions_get_dy(void) {
    return gs->conversions.y + GUI_H*2;
}

static inline int _gui_conversions_get_input_dx(void) {
    return 128;
}

static inline int _gui_conversions_get_fuel_dx_material(void) {
    return 216;
}

static inline int _gui_conversions_get_fuel_dy(void) {
    return 56;
}

static inline int _gui_conversions_get_output_offset(void) {
    return 96;
}

typedef struct GUIConversionsRects {
    SDL_Rect rects[99];
    int rect_count;
}GUIConversionsRects;
GUIConversionsRects gui_conversions_get_rect(Cell_Type type) {
    GUIConversionsRects result = {0};
    
    for (int i = 0; i < sizeof(conversions)/5; i++) {
        int converter = conversions[i*5];
        Cell_Type output = conversions[i*5+4];
        if (output != type) continue;
            
        int dy = 0;
        int dx;
        
        if (converter == CONVERTER_FUEL) {
            dx = _gui_conversions_get_fuel_dx();
            dy += i * _gui_conversions_get_jump_dy();
        } else {
            dx = _gui_conversions_get_material_dx();
            dy += (i-7) * _gui_conversions_get_jump_dy();
        }
        
        int pad = 64;
        
        result.rects[result.rect_count++] = (SDL_Rect){
            Scale(dx - pad),
            Scale(dy - pad),
            Scale(300),
            Scale(_gui_conversions_get_jump_dy()),
        };
    }
    return result;
}

void gui_conversions_draw_scroll_bar(int target) {
    Conversions *c = &gs->conversions;
    
    int w = Scale(16);
    int h = gs->window_height-GUI_H;
    
    c->scroll_bar_full = (SDL_Rect){
        gs->window_width-w,
        GUI_H,
        w,
        h,
    };
    
    int bar_height = Scale(64);
    
    c->scroll_bar = (SDL_Rect){
        gs->window_width-w,
        GUI_H - h*(c->y)/(c->max_height),
        w,
        bar_height
    };
    
    SDL_Point mouse = {gs->input.real_mx, gs->input.real_my};
    
    if (is_point_in_rect(mouse, c->scroll_bar) && gs->input.mouse_pressed[SDL_BUTTON_LEFT]) {
        c->holding_scroll_bar = true;
    }
    
    if (!(gs->input.mouse & SDL_BUTTON_LEFT)) {
        c->holding_scroll_bar = false;
    }
    
    if (c->holding_scroll_bar) {
        f64 dy = gs->input.real_my - gs->input.real_pmy;
        c->y += -(dy/(gs->window_height-GUI_H)) * c->max_height;
        f64 off = (f64)bar_height/(gs->window_height) * c->max_height;
        c->y = clamp(c->y, -c->max_height + off, 0);
        c->y_to = c->y;
    }
    
    RenderColor(32, 32, 32, 255);
    RenderFillRect(target, c->scroll_bar_full);
    RenderColor(255, 255, 255, 255);
    RenderFillRect(target, c->scroll_bar);
}

void converter_gui_draw(int final_target) {
    int target = RENDER_TARGET_CONVERSION_PANEL;
    
    gs->conversions.timer++;
    
    RenderMaybeSwitchToTarget(target);
    
    RenderColor(0,0,0,0);
    RenderClear(target);
    
    if (gs->input.keys_pressed[SDL_SCANCODE_I]) {
        gs->conversions.active = !gs->conversions.active;
    }
    
    if (!gs->conversions.active) return;
    
    gs->conversions.max_height = 3300;
    
    int dx = _gui_conversions_get_fuel_dx();
    int dy = _gui_conversions_get_dy();
    
    int cum = 0;
    
    int size = 48;
    
    gs->conversions.y = lerp64(gs->conversions.y, gs->conversions.y_to, 0.5);
    
    bool mouse_in_none = true;
    
    Font *title_font = gs->fonts.font_times;
    int a = 100;
    
    // Fuel conversions
    RenderTextQuick(target,
                    "fuelc",
                    title_font,
                    "Fuel Conversions",
                    WHITE,
                    255,
                    Scale(dx),
                    Scale(dy-a),
                    null,
                    null,
                    false);
    
    for (int i = 0; i < 7; i++) {
        int converter = conversions[i*5];
        Assert(converter == CONVERTER_FUEL);
        
        Cell_Type input1 = conversions[i*5+2];
        Cell_Type input2 = conversions[i*5+3];
        Cell_Type output = conversions[i*5+4];
        
        {
            int x = dx;
            int y = dy + cum;
            
            converter_gui_draw_text(target, "Input 1", x, y, size, i*4+2);
            bool mouse_in = converter_gui_item_draw(target,
                                                    input1,
                                                    x,
                                                    y,
                                                    size,
                                                    size);
            if (mouse_in) mouse_in_none = false;
        }
        
        {
            int x = dx + _gui_conversions_get_input_dx();
            int y = dy + cum;
            
            converter_gui_draw_text(target, "Input 2", x, y, size, i*4+3);
            bool mouse_in = converter_gui_item_draw(target,
                                                    input2,
                                                    x,
                                                    y,
                                                    size,
                                                    size);
            if (mouse_in) mouse_in_none = false;
        }
        
        {
            int x = ((dx)+(dx+_gui_conversions_get_input_dx()))/2;
            int y = dy + _gui_conversions_get_output_offset() + cum;
            
            { // Arrow
                RenderColor(255,255,255,255);
                int xx = Scale(dx + size/2) + Scale(dx + _gui_conversions_get_input_dx() + size/2);
                xx /= 2;
                
                int yy = Scale(dy+cum+20);
                
                converter_draw_arrow(target, xx, yy);
            }
            
            converter_gui_draw_text(target, "Output", x, y, size, i*4+4);
            bool mouse_in = converter_gui_item_draw(target,
                                                    output,
                                                    x,
                                                    y,
                                                    size,
                                                    size);
            if (mouse_in) mouse_in_none = false;
        }
        
        cum += _gui_conversions_get_jump_dy();
    }
    
    // Material conversions
    dx = _gui_conversions_get_material_dx();
    cum = 0;
    
    RenderTextQuick(target,
                    "matc",
                    title_font,
                    "Material Conversions",
                    WHITE,
                    255,
                    Scale(dx),
                    Scale(dy-a),
                    null,
                    null,
                    false);
    
    for (int i = 7; i <= 22; i++) {
        int idx = i*5;
        int converter = conversions[i*5];
        Assert(converter == CONVERTER_MATERIAL);
        
        Cell_Type fuel = conversions[idx+1];
        Cell_Type input1 = conversions[idx+2];
        Cell_Type input2 = conversions[idx+3];
        Cell_Type output = conversions[idx+4];
        
        
        {
            int x = dx + _gui_conversions_get_fuel_dx_material();
            int y = dy + cum + _gui_conversions_get_fuel_dy();
            
            converter_gui_draw_text(target, "Fuel", x, y, size, idx+1);
            bool mouse_in = converter_gui_item_draw(target,
                                                    fuel,
                                                    x,
                                                    y,
                                                    size,
                                                    size);
            if (mouse_in) mouse_in_none = false;
        }
        
        {
            int x = dx;
            int y = dy + cum;
            
            converter_gui_draw_text(target, "Input 1", x, y, size, idx+2);
            bool mouse_in = converter_gui_item_draw(target,
                                                    input1,
                                                    x,
                                                    y,
                                                    size,
                                                    size);
            if (mouse_in) mouse_in_none = false;
        }
        
        {
            int x = dx + _gui_conversions_get_input_dx();
            int y = dy + cum;
            
            converter_gui_draw_text(target, "Input 2", x, y, size, idx+3);
            bool mouse_in = converter_gui_item_draw(target,
                                                    input2,
                                                    x,
                                                    y,
                                                    size,
                                                    size);
            if (mouse_in) mouse_in_none = false;
        }
        
        {
            int x = ((dx)+(dx+_gui_conversions_get_input_dx()))/2;
            int y = dy +  _gui_conversions_get_output_offset() + cum;
            
            { // Arrow
                RenderColor(255,255,255,255);
                int xx = Scale(dx + size/2) + Scale(dx + _gui_conversions_get_input_dx() + size/2);
                xx /= 2;
                
                int yy = Scale(dy+cum+20);
                
                converter_draw_arrow(target, xx, yy);
            }
            
            converter_gui_draw_text(target, "Output", x, y, size, idx+4);
            bool mouse_in = converter_gui_item_draw(target,
                                                    output,
                                                    x,
                                                    y,
                                                    size,
                                                    size);
            if (mouse_in) mouse_in_none = false;
        }
        
        cum += _gui_conversions_get_jump_dy();
    }
    
    if (mouse_in_none) {
        tooltip_reset(&gs->gui.tooltip);
    }
    
    
    Conversions *c = &gs->conversions;
    if (c->definition_alpha) {
        int type = c->definition;
        
        GUIConversionsRects r = gui_conversions_get_rect(type);
        
        if (c->definition_alpha == 255) { // First frame clicked?
            // Set up the scroll position
            // Find the average of all the rects' y positions.
            
            f64 avg = 0;
            for (int i = 0; i < r.rect_count; i++) {
                avg += r.rects[i].y + r.rects[i].h/2;
            }
            avg /= r.rect_count;
            
            c->y_to = -avg + (gs->window_height-GUI_H)/2;
        }
        
        RenderColor(255, 0, 0, c->definition_alpha);
        for (int i = 0; i < r.rect_count; i++) {
            r.rects[i].y += c->y;
            RenderDrawRect(target, r.rects[i]);
        }
        c->definition_alpha--;
        
        if (c->definition_alpha <= 0) {
            c->definition_alpha = 0;
            c->definition = 0;
        }
    }
    
    
    gui_conversions_draw_scroll_bar(target);
    
    SDL_Rect src = {
        0, GUI_H,
        gs->window_width,
        gs->window_height-GUI_H
    };
    
    SDL_Rect dst = {
        0,
        GUI_H,
        gs->window_width,
        gs->window_height-GUI_H
    };
    
    RenderTargetToTarget(final_target,
                         target,
                         &src,
                         &dst);
}