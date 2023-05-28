static void effect_reset_snow(bool big) {
    for (int i = 0; i < gs->current_effect.particle_count; i++) {
        Effect_Particle *particle = &gs->current_effect.particles[i];
        particle->x = (f32) (rand()%gs->current_effect.w);
        particle->y = (f32) (rand()%gs->current_effect.h);
        
        
        if(!big) {
            f32 r1 = randf(0.5);
            f32 r2 = randf(1.0);
            particle->vx = 0.3f + r1*r1*r1;
            particle->vy = 0.3f + r2*r2*r2;
        } else {
            f32 r1 = randf(1.0);
            f32 r2 = randf(1.0);
            particle->vx = 0.3f + 0.5*r1*r1*r1*r1;
            particle->vy = 0.3f + r2*r2*r2*r2;
        }
        
        f64 spd = 0.3f;
        if (big) {
            spd = EFFECT_SCALE;
        }
        
        particle->vx *= spd;
        particle->vy *= spd;
    }
}

void effect_set(int type, int w, int h) {
    gs->current_effect.type = type;
    
    gs->current_effect.w = w;
    gs->current_effect.h = h;
    
    switch (type) {
        case EFFECT_NONE: {
            gs->current_effect.particles = NULL;
            return;
        }
        case EFFECT_SNOW: {
            gs->current_effect.particle_count = 200;
            if (w > gs->gw) {
                gs->current_effect.particle_count = 650;
            }
            break;
        }
        case EFFECT_RAIN: {
            gs->current_effect.particle_count = 50;
            break;
        }
    }
    
    if (gs->current_effect.particles == NULL) {
        gs->current_effect.particles = PushArray(gs->persistent_memory, gs->current_effect.particle_count, sizeof(Effect_Particle));
    }
    
    switch (type) {
        case EFFECT_SNOW: {
            effect_reset_snow(w > gs->gw);
            break;
        }
        case EFFECT_RAIN: {
            for (int i = 0; i < gs->current_effect.particle_count; i++) {
                Effect_Particle *particle = &gs->current_effect.particles[i];
                particle->x = (f32) (rand()%gs->gw);
                particle->y = (f32) (rand()%gs->gh);
                
                particle->vx = 0.3f + randf(0.3f);
                particle->vy = 1.f;
            }
            break;
        }
    }
}

void particle_tick(Effect *effect, int i) {
    if (gs->levels[gs->level_current].state == LEVEL_STATE_OUTRO)
        return;
    if (gs->paused && !gs->step_one)
        return;
    if (effect->type == EFFECT_NONE)
        return;
    
    switch (effect->type) {
        case EFFECT_SNOW: case EFFECT_RAIN: {
            Effect_Particle *particle = &gs->current_effect.particles[i];
            
            int reverse = (gs->level_current+1 >= 8) ? -1 : 1;
            
            particle->x += reverse * particle->vx;
            particle->y += reverse * particle->vy;
            
            if (effect->type == EFFECT_RAIN) {
                particle->vx += 0.003f * sinf(SDL_GetTicks() / 1000.f);
            }
            
#if 0
            if (gs->levels[gs->level_current].state == LEVEL_STATE_PLAY) {
                if (is_in_bounds((int)particle->x, (int)particle->y) && gs->grid[(int)particle->x + ((int)particle->y)*gs->gw].type) {
                    particle->x = (f32) (rand()%effect->w);
                    particle->y = 0;
                }
            }
#endif
            
            if (particle->x < 0) particle->x = (f32) effect->w-1;
            if (particle->y < 0) particle->y = (f32) effect->h-1;
            
            if (particle->x-particle->vx*3 > effect->w-1) particle->x = 0;
            if (particle->y-particle->vy*3 > effect->h-1) particle->y = 0;
            break;
        }
        default: break;
    }
}

void effect_draw(Effect *effect, bool draw_points, int only_slow) {
    if (effect->type == EFFECT_NONE)
        return;
    
#ifndef ALASKA_RELEASE_MODE
    if (gs->input.keys_pressed[SDL_SCANCODE_T]) {
        effect_reset_snow(false);
    }
#endif
    
    switch (effect->type) {
        case EFFECT_SNOW: {
            for (int i = 0; i < effect->particle_count; i++) {
                Effect_Particle *particle = &gs->current_effect.particles[i];
                f32 length = (f32) sqrt(particle->vx*particle->vx + particle->vy*particle->vy);
                
                if (only_slow == ONLY_SLOW_SLOW && length > 3)  continue;
                if (only_slow == ONLY_SLOW_FAST && length <= 3) continue;
                
                particle_tick(effect, i);
                
                f32 max;
                
                f32 spd = 0.3f;
                if (effect->w > gs->gw) {
                    spd = EFFECT_SCALE;
                }
                max = sqrt((spd*spd*0.8*0.8) + (spd*spd*1.3*1.3));
                
                int px = (int)particle->x;
                int py = (int)particle->y;
                
                //SDL_SetRenderDrawColor(gs->renderer, my_rand(px), my_rand(py), my_rand(px*py), (Uint8) (255 * (length/max)));
                float coeff = 0.4f;
                if (effect->w > gs->gw) coeff = 0.6f;
                
                Uint8 col = (Uint8) (255 * coeff*length/max);
                SDL_SetRenderDrawColor(gs->renderer, col, col, col, 255);
                
                if (draw_points) {
                    SDL_RenderDrawPoint(gs->renderer, px, py);
                } else {
                    
                    if (gs->levels[gs->level_current].state == LEVEL_STATE_NARRATION) {
                        fill_circle(gs->renderer,
                                    gs->window_width * px/effect->w,
                                    gs->window_height * py/effect->h,
                                    Scale(6 * (length/max)));
                    }
                }
            }
            break;
        }
        case EFFECT_RAIN: {
            for (int i = 0; i < effect->particle_count; i++) {
                particle_tick(effect, i);
                
                Effect_Particle *particle = &gs->current_effect.particles[i];
                
                int px = (int)particle->x;
                int py = (int)particle->y;
                
                int p2x = (int) (px - particle->vx*3);
                int p2y = (int) (py - particle->vy*3);
                
                SDL_SetRenderDrawColor(gs->renderer, 255, 255, 255, 32);
                SDL_RenderDrawLine(gs->renderer, px, py, p2x, p2y);
            }
            break;
        }
        default: break;
    }
}
