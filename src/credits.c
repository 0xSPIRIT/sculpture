void credits_run(void) {
    struct Credits *c = &gs->credits;
    
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
            f64 f = 20.f;
            
            if (c->timer < f)
                c->timer++;
            
            Uint8 co = 255 * (c->timer/f);
            SDL_Color col = {
                255-co, 255-co, 255-co, 255
            };
            
            draw_text(gs->fonts.font_consolas,
                      "Created by spiritwolf",
                      col,
                      WHITE,
                      true,
                      true,
                      128+gs->window_width/2,
                      2*gs->window_height/3,
                      NULL,
                      NULL);
            break;
        }
    }
}