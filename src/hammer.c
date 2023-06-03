static Hammer hammer_init(void) {
    Hammer hammer = {0};
    
    hammer.tex = &GetTexture(TEXTURE_CHISEL_HAMMER);
    hammer.dir = 1;
    
    return hammer;
}

static void hammer_tick(Hammer *hammer) {
    if (gs->input.keys[SDL_SCANCODE_RSHIFT]) {
        f64 rmx = (f64)gs->input.real_mx / (f64)gs->S;
        f64 rmy = (f64)(gs->input.real_my-GUI_H) / (f64)gs->S;
        
        hammer->angle = 180 + 360 * atan2f(rmy - hammer->y, rmx - hammer->x) / (f32)(2*M_PI);
        hammer->temp_angle = hammer->angle;
    } else {
        f64 dist = gs->chisel->texture->width;
        
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

// How the drawing works:
//
// Firstly, the local hammer's angle (swing animation)
// is drawn to a texture.
//
// Then, another texture is used to actually rotate the
// entire hammer to the chisel's angle, and position
// it accordingly.
//
static void hammer_draw(int final_target, Hammer *hammer) {
    {
        const int target = RENDER_TARGET_HAMMER;
        SDL_Point center = {
            hammer->tex->width/2,
            7*hammer->tex->height/8.0
        };
        
        SDL_Rect dst = {
            hammer->x + 1,
            hammer->y - 2,
            hammer->tex->width, hammer->tex->height
        };
        
        RenderColor(0, 0, 0, 0);
        RenderClear(target);
        
        SDL_RendererFlip flip = SDL_FLIP_NONE;
        
        f64 angle = hammer->angle;
        
        if (gs->chisel->angle < 90 && gs->chisel->angle > -90) {
            flip |= SDL_FLIP_VERTICAL;
            dst.y -= hammer->tex->height - 4;
            center.y -= hammer->tex->height - 4;
            angle *= -1;
        }
        
        RenderTextureEx(target,
                        hammer->tex,
                        NULL,
                        &dst,
                        angle,
                        &center,
                        flip);
    }
    
    // Now we render the target.
    
    {
        const int target = RENDER_TARGET_HAMMER2;
        
        RenderColor(0, 0, 0, 0);
        RenderClear(target);
        
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
        
        RenderTextureEx(target,
                        &RenderTarget(RENDER_TARGET_HAMMER)->texture,
                        NULL,
                        NULL,
                        180+gs->chisel->angle,
                        &center,
                        SDL_FLIP_NONE);
        
        RenderTexture(final_target,
                      &RenderTarget(target)->texture,
                      &src,
                      &dst);
    }
}