static void rain_splash(Rain_Splash *rain, int amount, int x, int y) {
    if (rain->splash_count + amount > MAX_SPLASH) return;
    
    for (int i = 0; i < amount; i++) {
        f32 vx = randf(2.0) - 1;
        f32 vy = -randf(2.0);
        
        f32 speed = 0.5;
        
        vx *= speed;
        vy *= speed;
        
        rain->splashes[rain->splash_count++] = (Effect_Particle){ x, y, vx, vy };
    }
}

static void draw_rain_splashes(int target, Rain_Splash *rain) {
    int sub_target = RENDER_TARGET_EFFECTS;
    
    RenderColor(0,0,0,0);
    RenderClear(sub_target);

    bool should_tick = !gs->paused || gs->step_one;
    
    for (int i = rain->splash_count-1; i >= 0; i--) {
        Effect_Particle *p = &rain->splashes[i];
        
        int col = 255;
        RenderColor(col, col, col, 255);
        RenderPointRelative(sub_target, p->x, p->y);

        int int_x = p->x;
        int int_y = p->y;
        
        if (should_tick) {
            p->x += p->vx;
            p->y += p->vy;

            int_x = p->x;
            int_y = p->y;
        
            p->vy += 0.1; // gravity
        }
        
        if (p->y > gs->gh || gs->grid[int_x+int_y*gs->gw].type) {
            rain->splash_count--;
            for (int j = i; j < rain->splash_count; j++)
                rain->splashes[j] = rain->splashes[j+1];
        }
    }
    
    RenderTextureAlphaMod(&RenderTarget(sub_target)->texture, 40);
    RenderTargetToTarget(target, sub_target, null, null);
}

static void reset_snow_particle(Effect *effect, Effect_Particle *particle) {
    SDL_Rect bounds = effect->bounds;
    
    particle->x = (f32) (bounds.x+(rand()%(bounds.w-bounds.x)));
    particle->y = (f32) (bounds.y+(rand()%(bounds.h-bounds.y)));
    
    f32 r1 = randf(0.5);
    f32 r2 = randf(1.0);
    particle->vx = 0.3f + r1*r1*r1;
    particle->vy = 0.3f + r2*r2*r2;

    const f64 spd = 0.3f;
    particle->vx *= spd;
    particle->vy *= spd;
}

static void reset_rain_particle(Effect *effect, Effect_Particle *particle) {
    SDL_Rect bounds = effect->bounds;

    particle->x = (f32) (bounds.x+(rand()%(bounds.w-bounds.x)));
    particle->y = (f32) (bounds.y+(rand()%(bounds.h-bounds.y)));
    
    // Favor slower particles than faster ones.
    
    f32 r1 = randf(0.5)+0.5f;
    
    r1 *= r1;

    particle->vx = r1 * 0.5f;
    particle->vy = r1 * 1.9f;
}

static void effect_reset_snow(Effect *effect, bool high_fidelity) {
    for (int i = 0; i < effect->particle_count; i++) {
        Effect_Particle *particle = &effect->particles[i];

        SDL_Rect bounds = effect->bounds;

        particle->x = (f32) (bounds.x+(rand()%(bounds.w-bounds.x)));
        particle->y = (f32) (bounds.y+(rand()%(bounds.h-bounds.y)));

        if (high_fidelity) {
            f32 r1 = randf(1.0);
            f32 r2 = randf(1.0);
            particle->vx = EFFECT_SCALE * (0.3f + 0.5*r1*r1*r1*r1);
            particle->vy = EFFECT_SCALE * (0.3f + r2*r2*r2*r2);
        } else {
            reset_snow_particle(effect, particle);
        }
    }
}

static Cell_Type effect_picked_up(Effect *effect) {
    if (effect->type == EFFECT_NONE) return CELL_NONE;

    if (effect->type == EFFECT_SNOW || effect->type == EFFECT_RAIN) {
        return CELL_WATER;
    }

    return CELL_NONE;
}

static void effect_handle_placer(Effect *effect, int x, int y, int r) {
    // Loop through all of the particles and check if they
    // are within our circle.
    Cell_Type effect_pickup = effect_picked_up(&gs->current_effect);

    if (!effect_pickup) return;
    Placer *current = get_current_placer();

    if (current->contains->type == 0 || current->contains->type == effect_pickup) {
        // good
    } else {
        return;
    }

    for (int i = effect->particle_count; i > 0; i--) {
        Effect_Particle *p = &effect->particles[i];
        if (distance(x, y, p->x, p->y) <= r) {
            // Make it look like we're removing it but really
            // just make it spawn somewhere else.
            switch (effect->type) {
                case EFFECT_RAIN: { reset_rain_particle(effect, p); } break;
                case EFFECT_SNOW: { reset_snow_particle(effect, p); } break;
            }
            get_current_placer()->contains->type = effect_pickup;
            get_current_placer()->contains->amount += 10;
        }
    }
}

static void effect_set(Effect *effect, Effect_Type type, bool high_fidelity, int x, int y, int w, int h) {
    effect->type = type;
    effect->high_fidelity = high_fidelity;

    effect->bounds = (SDL_Rect){x, y, w, h};

    switch (type) {
        case EFFECT_NONE: {
            effect->particles = null;
            return;
        }
        case EFFECT_SNOW: {
            effect->particle_count = 200*3;
            if (high_fidelity) {
                effect->particle_count = 650;
            }
            break;
        }
        case EFFECT_RAIN: {
            effect->particle_count = 500;
            break;
        }
    }

    if (effect->particles == null) {
        effect->particles = PushArray(gs->persistent_memory, effect->particle_count, sizeof(Effect_Particle));
    }

    switch (type) {
        case EFFECT_SNOW: {
            effect_reset_snow(effect, high_fidelity);
            break;
        }
        case EFFECT_RAIN: {
            for (int i = 0; i < effect->particle_count; i++) {
                Effect_Particle *particle = &effect->particles[i];
                reset_rain_particle(effect, particle);
            }
            break;
        }
    }
}

// If the length of velocity^2 of a particle is > this value,
// then it is in the "foreground" and can do things like
// generate splashes, or be obstructed by cells on the grid.
static f32 _get_particle_square_limit(int effect_type) {
    f32 result = 0;
    if (effect_type == EFFECT_SNOW) result = 0.2*0.2;
    if (effect_type == EFFECT_RAIN) result = 1.0*1.0;
    
    return result;
}

// returns if the particle collided with the ground, and had to be reset
typedef struct {
    bool hit;
    f32 x, y;
} ParticleSplashResult;

static ParticleSplashResult particle_tick(Effect *effect, int i) {
    ParticleSplashResult result = {0};
    
    if (gs->levels[gs->level_current].state == LEVEL_STATE_OUTRO)
        return result;
    if (gs->paused && !gs->step_one)
        return result;
    if (effect->type == EFFECT_NONE)
        return result;
    
    switch (effect->type) {
        case EFFECT_SNOW: case EFFECT_RAIN: {
            Effect_Particle *particle = &effect->particles[i];

            int reverse = (gs->level_current+1 == 11) ? -1 : 1;
            
            f32 prev_x = particle->x;
            f32 prev_y = particle->y;

            particle->x += reverse * particle->vx;
            particle->y += reverse * particle->vy;
            
            int int_px = (int)particle->x;
            int int_py = (int)particle->y;
            
            f32 sq_length = particle->vx*particle->vx + particle->vy*particle->vy;
            
            f32 limit = _get_particle_square_limit(effect->type);
            
            bool hit_cell;
            if (effect->high_fidelity) {
                hit_cell = false; // we don't deal with that in high fidelity narration scenes
            } else {
                hit_cell = (gs->grid[int_px+int_py*gs->gw].type != CELL_NONE);
            }
            
            if (is_in_bounds(int_px, int_py) &&
                sq_length > limit &&
                hit_cell &&
                gs->level_current+1 != 11)
           {
                if (effect->type == EFFECT_RAIN) {
                    // Go through every pixel from (prev_x, prev_y)
                    // to (particle->x, particle->y) to find the actual contact point
                    
                    f32 dx = particle->x - prev_x;
                    f32 dy = particle->y - prev_y;
                    
                    f32 length = sqrtf(dx*dx + dy*dy);
                    
                    dx /= length;
                    dy /= length;
                    
                    f32 x, y;
                    
                    for (x = prev_x, y = prev_y;
                         x < particle->x && y < particle->y;
                         x += dx, y += dy)
                    {
                        int ix = (int)x;
                        int iy = (int)y;
                        
                        if (!is_in_bounds(ix, iy)) break;
                        if (gs->grid[ix+iy*gs->gw].type != CELL_NONE) break;
                    }
                    
                    result.hit = true;
                    result.x = x;
                    result.y = y;
                }
                
                Log("%d, %d\n", effect->bounds.x, effect->bounds.x + effect->bounds.w);
                particle->x = effect->bounds.x + rand()%(effect->bounds.w-effect->bounds.x);
                particle->y = effect->bounds.y;
            }
            
            if (particle->x < effect->bounds.x)
                particle->x = (f32) effect->bounds.x+effect->bounds.w-1;
            
            if (reverse == -1) {
                if (particle->y < effect->bounds.y)
                    particle->y = (f32) effect->bounds.y+effect->bounds.h+particle->vy*3;
            } else {
                if (particle->y-particle->vy*3 > effect->bounds.y+effect->bounds.h-1) {
                    particle->y = effect->bounds.y;
                    
                    result.y = gs->gh-1;
                    result.x = particle->x;
                    result.hit = true;
                }
            }
            
            if (particle->x-particle->vx*3 > effect->bounds.x+effect->bounds.w-1)
                particle->x = effect->bounds.x;
            
            break;
        }
        default: break;
    }
    
    return result;
}

static void effect_draw(int target, Effect *effect, bool draw_points) {
    if (effect->type == EFFECT_NONE)
        return;
    
#ifndef ALASKA_RELEASE_MODE
    if (gs->input.keys_pressed[SDL_SCANCODE_T]) {
        effect_reset_snow(effect, false);
    }
#endif
    
    switch (effect->type) {
        case EFFECT_SNOW: {
            for (int i = 0; i < effect->particle_count; i++) {
                Effect_Particle *particle = &effect->particles[i];
                f32 length = (f32) sqrt(particle->vx*particle->vx + particle->vy*particle->vy);
                
                particle_tick(effect, i);
                
                if (!effect->high_fidelity) {
                    if (particle->x < 32 + gs->render.view.x/gs->S) continue;
                    if (particle->x > 32 + 64 + gs->render.view.x/gs->S) continue;
                }
                
                f32 max;
                
                f32 spd = 0.3f;
                if (effect->high_fidelity) {
                    spd = EFFECT_SCALE;
                }
                max = sqrt((spd*spd*0.8*0.8) + (spd*spd*1.3*1.3));
                
                int px = (int)particle->x;
                int py = (int)particle->y;
                
                f32 coeff = 0.4f;
                if (effect->bounds.w > gs->gw) coeff = 0.6f;
                
                Uint8 col = (Uint8) (255 * coeff*length/max);
                
                if (draw_points) {
                    RenderColor(255, 255, 255, col);
                    RenderPointRelative(target, px, py);
                } else {
                    RenderColor(col, col, col, 255);
                    int size = Scale(6*(length/max));
                    RenderFillCircle(target,
                                     gs->game_width * px/effect->bounds.w,
                                     gs->game_height * py/effect->bounds.h,
                                     size);
                }
            }
            break;
        }
        case EFFECT_RAIN: {
            int sub_target = RENDER_TARGET_EFFECTS;
            
            RenderColor(0,0,0,0);
            RenderClear(sub_target);
            
            for (int i = 0; i < effect->particle_count; i++) {
                Effect_Particle *p = &effect->particles[i];
                
                f32 length = p->vx * p->vx + p->vy * p->vy;
                
                int px = (int)p->x;
                int py = (int)p->y;
                
                ParticleSplashResult splash = particle_tick(effect, i);
                
                if (gs->level_current+1 == 11) splash.hit = false;
                
                if (splash.hit && length > _get_particle_square_limit(effect->type) && rand()<RAND_MAX/7) {
                    rain_splash(&effect->rain, 7, splash.x, splash.y);
                }

                // Updated from the particle_tick()
                px = (int)p->x;
                py = (int)p->y;

                int p2x = (int) (px - p->vx*3);
                int p2y = (int) (py - p->vy*3);
                
                {
                    f32 normalized = length / 3.854; // magic
                    normalized /= 2;
                    normalized += 0.5;
                    
                    Uint8 alpha = normalized * 255;
                    RenderColor(255, 255, 255, alpha);
                }
                
                RenderLineRelative(sub_target, px, py, p2x, p2y);
            }
            
            RenderTextureAlphaMod(&RenderTarget(sub_target)->texture, 40);
            RenderTargetToTarget(target, sub_target, null, null);
            break;
        }
        default: break;
    }
}
