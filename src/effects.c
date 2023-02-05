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
            gs->current_effect.particle_count = 100;
            if (w > gs->gw) {
                gs->current_effect.particle_count = 250;
            }
            break;
        }
        case EFFECT_RAIN: {
            gs->current_effect.particle_count = 50;
            break;
        }
    }
    
    if (gs->current_effect.particles == NULL) {
        gs->current_effect.particles = PushArray(gs->persistent_memory, gs->current_effect.particle_count, sizeof(struct Effect_Particle));
    }
    
    switch (type) {
        case EFFECT_SNOW: {
            for (int i = 0; i < gs->current_effect.particle_count; i++) {
                struct Effect_Particle *particle = &gs->current_effect.particles[i];
                particle->x = (f32) (rand()%gs->current_effect.w);
                particle->y = (f32) (rand()%gs->current_effect.h);
                
                particle->vx = 0.3f + randf(0.5);
                particle->vy = 0.3f + randf(1.0);
                
                f64 spd = 0.3f;
                if (w > gs->gw) {
                    spd = EFFECT_SCALE;
                }
                
                particle->vx *= spd;
                particle->vy *= spd;
            }
            break;
        }
        case EFFECT_RAIN: {
            for (int i = 0; i < gs->current_effect.particle_count; i++) {
                struct Effect_Particle *particle = &gs->current_effect.particles[i];
                particle->x = (f32) (rand()%gs->gw);
                particle->y = (f32) (rand()%gs->gh);
                
                particle->vx = 0.3f + randf(0.3f);
                particle->vy = 1.f;
            }
            break;
        }
    }
}

void particle_tick(struct Effect *effect, int i) {
    if (gs->paused && !gs->step_one)
        return;
    if (effect->type == EFFECT_NONE)
        return;
    
    switch (effect->type) {
        case EFFECT_SNOW: case EFFECT_RAIN: {
            struct Effect_Particle *particle = &gs->current_effect.particles[i];
            particle->x += particle->vx;
            particle->y += particle->vy;
            
            if (effect->type == EFFECT_RAIN) {
                particle->vx += 0.003f * sinf(SDL_GetTicks() / 1000.f);
            }
            
            if (gs->levels[gs->level_current].state == LEVEL_STATE_PLAY) {
                if (is_in_bounds((int)particle->x, (int)particle->y) && gs->grid[(int)particle->x + ((int)particle->y)*gs->gw].type) {
                    particle->x = (f32) (rand()%effect->w);
                    particle->y = 0;
                }
            }
            
            if (particle->x < 0) particle->x = (f32) effect->w-1;
            if (particle->y < 0) particle->y = (f32) effect->h-1;
            
            if (particle->x-particle->vx*3 > effect->w-1) particle->x = 0;
            if (particle->y-particle->vy*3 > effect->h-1) particle->y = 0;
            break;
        }
        default: break;
    }
}

void effect_draw(struct Effect *effect, bool draw_points) {
    if (effect->type == EFFECT_NONE)
        return;
    
    switch (effect->type) {
        case EFFECT_SNOW: {
            for (int i = 0; i < effect->particle_count; i++) {
                particle_tick(effect, i);
                
                struct Effect_Particle *particle = &gs->current_effect.particles[i];
                f32 length = (f32) sqrt(particle->vx*particle->vx + particle->vy*particle->vy);
                
                f32 max;
                
                f32 spd = 0.3f;
                if (effect->w > gs->gw) {
                    spd = EFFECT_SCALE;
                }
                max = sqrt((spd*spd*0.8*0.8) + (spd*spd*1.3*1.3));
                
                int px = (int)particle->x;
                int py = (int)particle->y;
                
                //SDL_SetRenderDrawColor(gs->renderer, my_rand(px), my_rand(py), my_rand(px*py), (Uint8) (255 * (length/max)));
                if (effect->w > gs->gw) {
                    SDL_SetRenderDrawColor(gs->renderer, 255, 255, 255, (Uint8) (255 * (0.3f*length/max)));
                } else {
                    SDL_SetRenderDrawColor(gs->renderer, 255, 255, 255, (Uint8) (255 * (0.3f*length/max)));
                }
                
                if (draw_points) {
                    SDL_RenderDrawPoint(gs->renderer, px, py);
                } else {
                    fill_circle(gs->renderer, px, py, Scale(6 * (length/max)));
                }
            }
            break;
        }
        case EFFECT_RAIN: {
            for (int i = 0; i < effect->particle_count; i++) {
                particle_tick(effect, i);
                
                struct Effect_Particle *particle = &gs->current_effect.particles[i];
                
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
