static void tooltip_reset(Tooltip *tooltip) {
    if (tooltip->preview) {
        tooltip->preview->index=0;
    }
    memset(tooltip, 0, sizeof(Tooltip));
    tooltip->x = tooltip->y = -1;
}

static void tooltip_set_position_to_cursor(Tooltip *tooltip, int type) {
    Input *input = &gs->input;
    tooltip->x = (f32)input->real_mx/gs->S;
    tooltip->y = (f32)input->real_my/gs->S - GUI_H/gs->S;
    tooltip->type = type;
}

static void tooltip_set_position(Tooltip *tooltip, int x, int y, int type) {
    tooltip->x = (f32) x;
    tooltip->y = (f32) y;
    tooltip->type = type;
}

static void tooltip_draw_box(int target, Tooltip *tooltip, int w, int h) {
    tooltip->w = w;
    tooltip->h = h;
    
    SDL_Rect r = {
        (int) (tooltip->x * gs->S),
        (int) (tooltip->y * gs->S + GUI_H),
        w,
        h
    };
    
    RenderColor(12, 12, 12, 255);
    RenderFillRect(target, r);
    RenderColor(255, 255, 255, 255);
    RenderDrawRect(target, r);
}

static void tooltip_get_string(int type, int amt, char *out_str) {
    char name[256] = {0};
    if (amt == 0) {
        strcpy(out_str, "Contains nothing");
        return;
    }
    get_name_from_type(type, name);
    sprintf(out_str, "Contains %s", name);
    if (amt) {
        char s[256] = {0};
        sprintf(s, " amt: %d", amt);
        strcat(out_str, s);
    }
}

static void tooltip_draw(int target, Tooltip *tooltip) {
    if (tooltip->x == -1 || tooltip->y == -1) return;
    
    int margin = 8; // In real pixels.
    
    int line_count = 0;
    SDL_Rect dsts[MAX_TOOLTIP_LINE_LEN]; // Destination rectangles
    
    int highest_w = 0;
    int cum_height = 0; // Cumulative height
    
    for (int i = 0; i < MAX_TOOLTIP_LINE_LEN; i++) {
        if (tooltip->str[i][0] == 0) continue;
        
        line_count++;
        
        int w, h;
        TTF_SizeText(gs->fonts.font->handle, tooltip->str[i], &w, &h);
        
        dsts[i] = (SDL_Rect){
            (int) (gs->S * tooltip->x + margin),
            GUI_H + (int) (cum_height + gs->S * tooltip->y + margin),
            w,
            h
        };
        
        cum_height += h;
        
        if (w > highest_w) {
            highest_w = w;
        }
    }
    
    int preview_scale = Scale(5);
    int old_h = cum_height;
    
    if (tooltip->preview) {
        cum_height += 64*preview_scale+margin*2;
        if (preview_scale*64 > highest_w) highest_w = preview_scale*64;
    }
    
    bool clamped = false;
    
    // Clamp the tooltips if it goes outside the window.
    if (tooltip->x*gs->S + highest_w >= gs->S*gs->gw) {
        tooltip->x -= highest_w/gs->S;
        for (int i = 0; i < line_count; i++)
            dsts[i].x -= highest_w - margin/2;
        clamped = true;
    }
    
    tooltip_draw_box(target,
                     tooltip,
                     highest_w + margin*2,
                     cum_height + margin*2);
    
    if (tooltip->preview) {
        preview_draw(target,
                     tooltip->preview,
                     gs->S*tooltip->x+margin,
                     gs->S*tooltip->y+(old_h)+margin*2,
                     preview_scale);
    }
    
    if (clamped) {
        tooltip->x += highest_w;
    }
    
    for (int i = 0; i < line_count; i++) {
        char identifier[64] = {0};
        sprintf(identifier, "tooltip %d", i);
        
        SDL_Color text_color;
        
        if (i == 0) {
            text_color = (SDL_Color){200,200,200,255};
        } else {
            text_color = WHITE;
        }
        
        RenderDrawTextQuick(target,
                            identifier,
                            gs->fonts.font,
                            tooltip->str[i],
                            text_color,
                            255,
                            dsts[i].x,
                            dsts[i].y,
                            NULL,
                            NULL,
                            false);
    }
}