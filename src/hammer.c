Hammer hammer_init(void) {
    Hammer hammer = {0};
    
    hammer.tex = Texture(TEXTURE_CHISEL_HAMMER);
    hammer.dir = 1;
    SDL_QueryTexture(hammer.tex, NULL, NULL, &hammer.w, &hammer.h);
    
    return hammer;
}

void hammer_tick(Hammer *hammer) {
    if (gs->input.keys[SDL_SCANCODE_RSHIFT]) {
        f64 rmx = (f64)gs->input.real_mx / (f64)gs->S;
        f64 rmy = (f64)(gs->input.real_my-GUI_H) / (f64)gs->S;
        
        hammer->angle = 180 + 360 * atan2f(rmy - hammer->y, rmx - hammer->x) / (f32)(2*M_PI);
        hammer->temp_angle = hammer->angle;
    } else {
        f64 dist = gs->chisel->w;
        
        hammer->x = gs->chisel->x + dist;
        hammer->y = gs->chisel->y;
    }
    
    if (gs->input.real_my > GUI_H &&
        !gs->tutorial.active &&
        !gs->gui.popup &&
        gs->input.mouse_pressed[SDL_BUTTON_LEFT] &&
        gs->chisel->highlight_count > 0 &&
        (hammer->state == HAMMER_STATE_IDLE ||
         hammer->state == HAMMER_STATE_BLOWBACK))
    {
        hammer->angle = hammer->temp_angle;
        hammer->t = 0;
        
        hammer->state = HAMMER_STATE_WINDUP;
        hammer->temp_angle = hammer->angle;
    }
    
    const f32 speed = 2.f;
    
    switch (hammer->state) {
        case HAMMER_STATE_WINDUP: {
            hammer->angle += speed * hammer->dir * 6;
            
            if (fabs(hammer->angle - hammer->temp_angle) >= 60) {
                hammer->state = HAMMER_STATE_ATTACK;
            }
            break;
        }
        case HAMMER_STATE_ATTACK: {
            int p_sign = sign(hammer->angle - hammer->temp_angle);
            hammer->angle -= speed * hammer->dir * 16;
            
            if (p_sign != sign(hammer->angle - hammer->temp_angle)) {
                gs->chisel->state = CHISEL_STATE_CHISELING;
                hammer->state = HAMMER_STATE_BLOWBACK;
                hammer->t = -1;
            }
            break;
        }
        case HAMMER_STATE_BLOWBACK: {
            hammer->angle = hammer->temp_angle + hammer->dir * 16 * (1 - hammer->t*hammer->t);
            hammer->t += 0.15;
            
            if (hammer->t >= 1) {
                hammer->state = HAMMER_STATE_IDLE;
                hammer->angle = hammer->temp_angle;
            }
            break;
        }
    }
}

void hammer_draw(Hammer *hammer) {
    {
        SDL_Texture *old_target = SDL_GetRenderTarget(gs->renderer);
        
        SDL_SetRenderTarget(gs->renderer,
                            RenderTarget(RENDER_TARGET_HAMMER));
        
        SDL_Point center = {
            hammer->w/2,
            7*hammer->h/8.0
        };
        
        SDL_Rect dst = {
            hammer->x + 1,
            hammer->y - 2,
            hammer->w, hammer->h
        };
        
        SDL_SetRenderDrawColor(gs->renderer, 0, 0, 0, 0);
        SDL_RenderClear(gs->renderer);
        
        SDL_RendererFlip flip = SDL_FLIP_NONE;
        
        f64 angle = hammer->angle;
        
        if (gs->chisel->angle < 90 && gs->chisel->angle > -90) {
            flip |= SDL_FLIP_VERTICAL;
            dst.y -= hammer->h - 4;
            center.y -= hammer->h - 4;
            angle *= -1;
        }
        
        SDL_RenderCopyEx(gs->renderer,
                         hammer->tex,
                         NULL,
                         &dst,
                         angle,
                         &center,
                         flip);
        
        SDL_SetRenderTarget(gs->renderer, old_target);
    }
    
    // Now we render the target.
    
    {
        SDL_Texture *old_target = SDL_GetRenderTarget(gs->renderer);
        SDL_SetRenderTarget(gs->renderer, RenderTarget(RENDER_TARGET_HAMMER2));
        
        SDL_SetRenderDrawColor(gs->renderer, 0, 0, 0, 0);
        SDL_RenderClear(gs->renderer);
        
        SDL_Point center = {
            gs->chisel->x,
            gs->chisel->y
        };
        SDL_Rect src = {
            0, 0,
            gs->gw, gs->gh
        };
        SDL_Rect dst = {
            0, 0,
            gs->gw, gs->gh
        };
        
        SDL_RenderCopyEx(gs->renderer,
                         RenderTarget(RENDER_TARGET_HAMMER),
                         NULL,
                         NULL,
                         180+gs->chisel->angle,
                         &center,
                         SDL_FLIP_NONE);
        
        SDL_SetRenderTarget(gs->renderer, old_target);
        
        SDL_RenderCopy(gs->renderer, RenderTarget(RENDER_TARGET_HAMMER2), &src, &dst);
    }
}