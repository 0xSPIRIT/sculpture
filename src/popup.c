void set_text_field(const char *description, const char *initial_text, void (*on_return)(const char *)) {
    struct Text_Field *text_field = &gs->text_field;
    
    text_field->active = true;
    text_field->on_return = on_return;
    strcpy(text_field->description, description);
    memset(text_field->text, 0, 256);
    strcpy(text_field->text, initial_text);
}

void text_field_tick(void) {
    struct Text_Field *text_field = &gs->text_field;
    
    if (!text_field->active) return;
    
    switch (gs->event->type) {
        case SDL_TEXTINPUT:
        strcat(text_field->text, gs->event->text.text);
        break;
        case SDL_KEYDOWN:
        switch (gs->event->key.keysym.sym) {
            case SDLK_RETURN:
            text_field->on_return(text_field->text);
            text_field->active = false;
            memset(text_field->description, 0, 256);
            memset(text_field->text, 0, 256);
            break;
            case SDLK_BACKSPACE:;
            size_t length = strlen(text_field->text);
            if (length)
                text_field->text[length-1] = 0;
            break;
            case SDLK_ESCAPE:
            memset(text_field->description, 0, 256);
            memset(text_field->text, 0, 256);
            text_field->active = false;
            break;
        }
        break;
    }
}

void text_field_draw(void) {
    struct Text_Field *text_field = &gs->text_field;
    
    if (!text_field->active) return;
    
    SDL_Surface *description_surf = 0, *text_surf = 0;
    SDL_Texture *description_texture = 0, *text_texture = 0;
    
    SDL_Rect field_rect = {0}, text_field_rect = {0}, description_rect = {0};
    
    if (*text_field->description)
        description_surf = TTF_RenderText_LCD(gs->fonts.font_consolas, text_field->description, (SDL_Color){180,180,180,255}, BLACK);
    if (*text_field->text)
        text_surf = TTF_RenderText_LCD(gs->fonts.font_consolas, text_field->text, WHITE, BLACK);

    if (text_surf) {
        field_rect = (SDL_Rect){
            (gs->gw*gs->S)/2 - text_surf->w/2 - 16, GUI_H + (gs->gh*gs->S)/2 - text_surf->h/2 - 16,
            text_surf->w + 32, text_surf->h + 32
        };
        text_field_rect = (SDL_Rect){
            (gs->gw*gs->S)/2 - text_surf->w/2, GUI_H + (gs->gh*gs->S)/2 - text_surf->h/2,
            text_surf->w, text_surf->h
        };
    }
    SDL_SetRenderDrawColor(gs->renderer, 0, 0, 0, 255);

    if (text_surf)
        SDL_RenderFillRect(gs->renderer, &field_rect);

    if (description_surf)
        description_texture = SDL_CreateTextureFromSurface(gs->renderer, description_surf);
    if (text_surf)
        text_texture = SDL_CreateTextureFromSurface(gs->renderer, text_surf);

    SDL_SetRenderDrawColor(gs->renderer, 255, 255, 255, 255);
    
    if (text_surf) {
        SDL_RenderDrawRect(gs->renderer, &field_rect);
        SDL_RenderCopy(gs->renderer, text_texture, NULL, &text_field_rect);
    }

    if (description_surf) {
        int x = gs->gw*gs->S/2;
        int y = gs->gh*gs->S/2;
        int hh = 0;

        if (text_surf) {
            hh = field_rect.h;
        } else {
            hh = 32;
        }

        description_rect = (SDL_Rect){
            x - description_surf->w/2, GUI_H + y - hh,
            description_surf->w, description_surf->h
        };

        SDL_SetRenderDrawColor(gs->renderer, 0, 0, 0, 255);
        SDL_RenderFillRect(gs->renderer, &description_rect);
        SDL_SetRenderDrawColor(gs->renderer, 255, 255, 255, 255);
        SDL_RenderCopy(gs->renderer, description_texture, NULL, &description_rect);

        SDL_DestroyTexture(description_texture);
        SDL_FreeSurface(description_surf);
    }
}
