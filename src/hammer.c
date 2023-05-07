struct Hammer hammer_init(void) {
    struct Hammer hammer = {0};
    
    hammer.tex = gs->textures.chisel_hammer;
    hammer.dir = 1;
    SDL_QueryTexture(hammer.tex, NULL, NULL, &hammer.w, &hammer.h);
    
    return hammer;
}

void hammer_tick(struct Hammer *hammer) {
    if (gs->input.keys[SDL_SCANCODE_RSHIFT]) {
        f64 rmx = (f64)gs->input.real_mx / (f64)gs->S;
        f64 rmy = (f64)(gs->input.real_my-GUI_H) / (f64)gs->S;
        
        hammer->angle = 180 + 360 * atan2f(rmy - hammer->y, rmx - hammer->x) / (f32)(2*M_PI);
        hammer->temp_angle = hammer->angle;
    } else {
        f64 angle = M_PI * gs->chisel->angle / 180.0;
        f64 dist = gs->chisel->w;
        f64 dx = dist * cos(angle);
        f64 dy = dist * sin(angle);
        
        hammer->x = gs->chisel->x - dx;
        hammer->y = gs->chisel->y - dy;
    }
    
    if (gs->input.keys_pressed[SDL_SCANCODE_T] &&
        (hammer->state == HAMMER_STATE_IDLE ||
         hammer->state == HAMMER_STATE_BLOWBACK))
    {
        hammer->angle = hammer->temp_angle;
        hammer->t = 0;
        
        hammer->state = HAMMER_STATE_WINDUP;
        hammer->temp_angle = hammer->angle;
    }
    
    switch (hammer->state) {
        case HAMMER_STATE_WINDUP: {
            hammer->angle += hammer->dir * 8;
            if (fabs(hammer->angle - hammer->temp_angle) >= 64) {
                hammer->state = HAMMER_STATE_ATTACK;
            }
            break;
        }
        case HAMMER_STATE_ATTACK: {
            hammer->angle -= hammer->dir * 16;
            if (fabs(hammer->angle - hammer->temp_angle) <= 1) {
                hammer->state = HAMMER_STATE_BLOWBACK;
                hammer->t = -1;
            }
            break;
        }
        case HAMMER_STATE_BLOWBACK: {
            hammer->angle = hammer->temp_angle + hammer->dir * 16 * (1 - hammer->t*hammer->t);
            hammer->t += 0.1;
            if (hammer->t >= 1) {
                hammer->state = HAMMER_STATE_IDLE;
                hammer->angle = hammer->temp_angle;
            }
            break;
        }
    }
}

void hammer_draw(struct Hammer *hammer) {
    SDL_Point center = { hammer->w/2, 7*hammer->h/8.0 };
    
    SDL_Rect dst = {
        hammer->x,
        hammer->y,
        hammer->w, hammer->h
    };
    
    SDL_RenderCopyEx(gs->renderer,
                     hammer->tex,
                     NULL,
                     &dst,
                     hammer->angle,
                     &center,
                     SDL_FLIP_NONE);
}