static void credits_run(int target) {
    Credits *c = &gs->credits;

    if (c->state == CREDITS_OFF) return;

    switch (c->state) {
        case CREDITS_DELAY: {
            if (c->timer < 4*60) {
                c->timer++;
            } else {
                c->timer = 0;
                c->state = CREDITS_SHOW;
            }
            break;
        }
        case CREDITS_SHOW: {
            f64 f = 18.f;

            if (c->timer < f)
                c->timer++;

            u8 co = 255 * (c->timer/f);
            SDL_Color col = {
                255-co, 255-co, 255-co, 255
            };

            Render_Text_Data text_data = {0};
            strcpy(text_data.identifier, "Ending text");
            text_data.font = gs->fonts.font_times;
            strcpy(text_data.str, "Created by spiritwolf");
            text_data.x = gs->game_width/2;
            text_data.y = gs->game_height/2 - 100;
            text_data.foreground = col;
            text_data.background = WHITE;
            text_data.alignment = ALIGNMENT_CENTER;
            text_data.render_type = TEXT_RENDER_BLENDED;

            RenderText(target, &text_data);
            break;
        }
    }
}
