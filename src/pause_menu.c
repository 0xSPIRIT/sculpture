// Look at assign_channel_volumes for where the master volume is set
static void pause_menu_init(Pause_Menu *menu) {
    menu->active = false;
    menu->slider = INITIAL_VOLUME;
    //Mix_MasterVolume(MIX_MAX_VOLUME * menu->slider);
}

static void pause_menu_draw(int target, Pause_Menu *menu) {
    Render_Text_Data data = {0};
    
    strcpy(data.identifier, "pause");
    data.font = gs->fonts.font_title;
    data.foreground = WHITE;
    data.background = BLACK;
    data.alignment = ALIGNMENT_CENTER;
    data.render_type = TEXT_RENDER_BLENDED;
    data.x = gs->game_width/2;
    data.y = gs->game_height/5;
    strcpy(data.str, "Paused");
    
    RenderText(target, &data);
    
    int width = gs->game_width;
    int height = gs->game_height;
    
    f32 h = Scale(20);
    f32 w = Scale(20);
    
    const f32 size = gs->game_width * 0.5f;
    const f32 working_size = size - w;
        
    RenderColor(127, 127, 127, 255);
    RenderLine(target,
               width/2 - size/2,
               height/2,
               width/2 + size/2,
               height/2);
    RenderLine(target,
               width/2 - size/2,
               height/2 - h,
               width/2 - size/2,
               height/2 + h);
    RenderLine(target,
               width/2 + size/2,
               height/2 - h,
               width/2 + size/2,
               height/2 + h);
    
    RenderColor(255, 255, 255, 255);
    SDL_Rect rect = {
        menu->slider * (working_size) + width/2 - size/2,
        height/2 - h,
        w,
        h*2
    };
    RenderFillRect(target, rect);
    
    int mx = gs->input.real_mx, my = gs->input.real_my;
    
    if (gs->input.mouse_pressed[SDL_BUTTON_LEFT] &&
        mx >= rect.x &&
        my >= rect.y &&
        mx <= rect.x+rect.w &&
        my <= rect.y+rect.h)
    {
        menu->holding_slider = true;
    }
    
    if (!(gs->input.mouse & SDL_BUTTON(SDL_BUTTON_LEFT))) {
        menu->holding_slider = false;
    }
    
    if (menu->holding_slider) {
        int dx = gs->input.real_mx - gs->input.real_pmx;
#ifdef __EMSCRIPTEN__
        dx = gs->input.em_dx;
#endif
        //menu->slider = (f32)(mx - (width/2-size/2)) / working_size;
        menu->slider += (f32)dx / working_size;
        menu->slider = clampf(menu->slider, 0, 1);
        //Mix_MasterVolume(MIX_MAX_VOLUME * menu->slider);
    }
    
    RenderTextQuick(target,
                    "volume",
                    gs->fonts.font_times,
                    "Master Volume",
                    WHITE,
                    width/2 - size/2,
                    height/2 - Scale(70),
                    0, 0, false);
    
#ifndef __EMSCRIPTEN__
    {
        Render_Text_Data text_data = {0};
        
        SDL_Color col = (SDL_Color){200,200,200,255};
        
        strcpy(text_data.identifier, "close");
        text_data.font = gs->fonts.font_courier;
        strcpy(text_data.str, "Exit Game");
        text_data.foreground = col;
        text_data.background = BLACK;
        text_data.x = Scale(32);
        text_data.y = height - Scale(32);
        text_data.alignment = ALIGNMENT_BOTTOM_LEFT;
        text_data.render_type = TEXT_RENDER_LCD;
        
        RenderText(target, &text_data);
        
        rect = (SDL_Rect){text_data.x, text_data.y - text_data.texture.height, text_data.texture.width, text_data.texture.height};
        rect.x -= Scale(2);
        rect.y -= Scale(2);
        rect.w += Scale(4);
        rect.h += Scale(4);
        
        if (mouse_in_rect(rect)) {
            if (gs->input.mouse_pressed[SDL_BUTTON_LEFT])  {
                save_game();
                gs->close_game = true;
            } else {
                col = (SDL_Color){ 127, 127, 127, 255 };
            }
        }
        
        RenderColor(col.r, col.g, col.b, 255);
        RenderDrawRect(target, rect);
    }
#endif
    
    {
        Render_Text_Data text_data = {0};
        
        SDL_Color col = (SDL_Color){200,200,200,255};
        
        strcpy(text_data.identifier, "5close");
        text_data.font = gs->fonts.font_courier;
        strcpy(text_data.str, "Back to Game");
        text_data.foreground = col;
        text_data.background = BLACK;
        text_data.x = Scale(32);
        text_data.y = height - Scale(75);
        text_data.alignment = ALIGNMENT_BOTTOM_LEFT;
        text_data.render_type = TEXT_RENDER_LCD;
        
        RenderText(target, &text_data);
        
        rect = (SDL_Rect){text_data.x, text_data.y - text_data.texture.height, text_data.texture.width, text_data.texture.height};
        rect.x -= Scale(2);
        rect.y -= Scale(2);
        rect.w += Scale(4);
        rect.h += Scale(4);
        
        if (mouse_in_rect(rect)) {
            if (gs->input.mouse_pressed[SDL_BUTTON_LEFT])  {
                gs->pause_menu.active = !gs->pause_menu.active;
            } else {
                col = (SDL_Color){ 127, 127, 127, 255 };
            }
        }
        
        RenderColor(col.r, col.g, col.b, 255);
        RenderDrawRect(target, rect);
    }
}
