static void set_text_field(const char *description, const char *initial_text, void (*on_return)(const char *)) {
    Text_Field *text_field = &gs->text_field;

    text_field->active = true;
    text_field->on_return = on_return;
    strcpy(text_field->description, description);
    memset(text_field->text, 0, 256);
    strcpy(text_field->text, initial_text);
}

static void text_field_tick(void) {
    Text_Field *text_field = &gs->text_field;

    if (!text_field->active) return;

    switch (gs->event->type) {
        case SDL_TEXTINPUT: {
            if (gs->event->text.text[0] != '`')
                strcat(text_field->text, gs->event->text.text);
            break;
        }
        case SDL_KEYDOWN: {
            switch (gs->event->key.keysym.sym) {
                case SDLK_RETURN: {
                    text_field->on_return(text_field->text);
                    text_field->active = false;
                    memset(text_field->description, 0, 256);
                    memset(text_field->text, 0, 256);
                    break;
                }
                case SDLK_BACKSPACE: {
                    size_t length = strlen(text_field->text);
                    if (length)
                        text_field->text[length-1] = 0;
                    break;
                }
                case SDLK_ESCAPE: {
                    memset(text_field->description, 0, 256);
                    memset(text_field->text, 0, 256);
                    text_field->active = false;
                    break;
                }
            }
            break;
        }
    }
}

static void text_field_draw(int target) {
    Text_Field *text_field = &gs->text_field;

    if (!text_field->active) return;

    const int hh = 140;

    Render_Text_Data data_desc = {0};
    Render_Text_Data data_text = {0};

    { // Description
        strcpy(data_desc.identifier, "fielda");
        data_desc.font = gs->fonts.font_courier;
        strcpy(data_desc.str, text_field->description);
        data_desc.x = gs->game_width/2;
        data_desc.y = GUI_H + gs->game_height/2 - Scale(hh);
        data_desc.foreground = (SDL_Color){180,180,180,255};
        data_desc.alignment = ALIGNMENT_CENTER;
        data_desc.render_type = TEXT_RENDER_BLENDED;

        RenderText(target, &data_desc);
    }

    if (text_field->text[0]) {
        RenderColor(0, 0, 0, 255);

        int at_x = gs->game_width/2;
        int at_y = gs->game_height/2;

        int w, h;
        RenderSizeText(gs->fonts.font_courier, text_field->text, &w, &h);

        SDL_Rect field_rect = {
            at_x - w/2 - 16,
            at_y - h/2 - 16,
            w + 32,
            h + 32
        };

        RenderFillRect(target, field_rect);

        RenderColor(255, 255, 255, 255);
        RenderDrawRect(target, field_rect);

        // Text Surface
        strcpy(data_text.identifier, "fieldb");
        data_text.font = gs->fonts.font_courier;
        strcpy(data_text.str, text_field->text);
        data_text.x = at_x;
        data_text.y = at_y;
        data_text.foreground = WHITE;
        data_text.render_type = TEXT_RENDER_BLENDED;
        data_text.alignment = ALIGNMENT_CENTER;

        RenderText(target, &data_text);
    }
}
