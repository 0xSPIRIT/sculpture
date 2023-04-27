void tooltip_reset(struct Tooltip *tooltip) {
    if (tooltip->preview) {
        tooltip->preview->index=0;
    }
    memset(tooltip, 0, sizeof(struct Tooltip));
    tooltip->x = tooltip->y = -1;
}

void tooltip_set_position_to_cursor(struct Tooltip *tooltip, int type) {
    struct Input *input = &gs->input;
    tooltip->x = (f32)input->real_mx/gs->S;
    tooltip->y = (f32)input->real_my/gs->S - GUI_H/gs->S;
    tooltip->type = type;
}

void tooltip_set_position(struct Tooltip *tooltip, int x, int y, int type) {
    tooltip->x = (f32) x;
    tooltip->y = (f32) y;
    tooltip->type = type;
}

void tooltip_draw_box(struct Tooltip *tooltip, int w, int h) {
    tooltip->w = w;
    tooltip->h = h;

    SDL_Rect r = {
        (int) (tooltip->x * gs->S),
        (int) (tooltip->y * gs->S + GUI_H),
        w,
        h
    };
    
    SDL_SetRenderDrawColor(gs->renderer, 12, 12, 12, 255);
    SDL_RenderFillRect(gs->renderer, &r);
    SDL_SetRenderDrawColor(gs->renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(gs->renderer, &r);
}

void tooltip_get_string(int type, int amt, char *out_str) {
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

// This happens outside of the pixel-art texture, so we must
// multiply all positions by scale.
void tooltip_draw(struct Tooltip *tooltip) {
    if (tooltip->x == -1 && tooltip->y == -1) return;
    
    const int margin = 8; // In real pixels.
    
    SDL_Surface *surfaces[MAX_TOOLTIP_LINE_LEN] = {0};
    SDL_Texture *textures[MAX_TOOLTIP_LINE_LEN] = {0};
    SDL_Rect dsts[MAX_TOOLTIP_LINE_LEN];
    int count = 0;
    
    int highest_w = 0;
    int height = 0;
    
    for (int i = 0; i < MAX_TOOLTIP_LINE_LEN; i++) {
        if (!strlen(tooltip->str[i])) continue;
        count++;
        
        SDL_Color color;
        
        if (i == 0) {
            color = (SDL_Color){200,200,200,255};
        } else {
            color = WHITE;
        }

        // @Performance
        surfaces[i] = TTF_RenderText_Blended(gs->fonts.font,
                                         tooltip->str[i],
                                             color);
        
        Assert(surfaces[i]);
        textures[i] = SDL_CreateTextureFromSurface(gs->renderer, surfaces[i]);
        Assert(textures[i]);
        
        dsts[i] = (SDL_Rect){
            (int) (gs->S * tooltip->x + margin),
            (int) (height + gs->S * tooltip->y + margin),
            surfaces[i]->w,
            surfaces[i]->h
        };
        dsts[i].y += GUI_H;

        height += surfaces[i]->h;

        if (surfaces[i]->w > highest_w) highest_w = surfaces[i]->w;
    }
    
    const int s = 5;
    
    int old_h = height;
    
    if (tooltip->preview) {
        height += 64*s+margin*2;
        if (64*s > highest_w) highest_w = 64*s;
    }

    bool clamped = false;

    // Clamp the tooltips if it goes outside the window.
    if (tooltip->x*gs->S + highest_w >= gs->S*gs->gw) {
        tooltip->x -= highest_w/gs->S;
        for (int i = 0; i < count; i++)
            dsts[i].x -= highest_w - margin/2;
        clamped = true;
    }

    tooltip_draw_box(tooltip, highest_w + margin*2, height + margin*2);

    if (tooltip->preview)
        preview_draw(tooltip->preview,
                 gs->S*tooltip->x+margin,
                 gs->S*tooltip->y+(old_h)+margin*2, s);
    
    if (clamped) {
        tooltip->x += highest_w;
    }

    for (int i = 0; i < count; i++) {
        SDL_RenderCopy(gs->renderer, textures[i], NULL, &dsts[i]);
    }

    for (int i = 0; i < MAX_TOOLTIP_LINE_LEN; i++) {
        if (surfaces[i]) {
            SDL_FreeSurface(surfaces[i]);
        }
        if (textures[i]) {
            SDL_DestroyTexture(textures[i]);
        }
    }
}
