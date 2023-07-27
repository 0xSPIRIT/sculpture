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
    
    particle->vx = 0.3f;
    particle->vy = 1.f;
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
            effect->particle_count = 200;
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

static void particle_tick(Effect *effect, int i) {
    if (gs->levels[gs->level_current].state == LEVEL_STATE_OUTRO)
        return;
    if (gs->paused && !gs->step_one)
        return;
    if (effect->type == EFFECT_NONE)
        return;

    switch (effect->type) {
        case EFFECT_SNOW: case EFFECT_RAIN: {
            Effect_Particle *particle = &effect->particles[i];

            int reverse = (gs->level_current+1 == 11) ? -1 : 1;

            particle->x += reverse * particle->vx;
            particle->y += reverse * particle->vy;

            if (effect->type == EFFECT_RAIN) {
                //particle->vx += 0.003f * sinf(SDL_GetTicks() / 1000.f);
            }

            if (particle->x < effect->bounds.x)
                particle->x = (f32) effect->bounds.w-1;
            if (particle->y < effect->bounds.y)
                particle->y = (f32) effect->bounds.h-1;

            if (particle->x-particle->vx*3 > effect->bounds.w-1)
                particle->x = effect->bounds.x;
            if (particle->y-particle->vy*3 > effect->bounds.h-1)
                particle->y = effect->bounds.y;

            break;
        }
        default: break;
    }
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

                //RenderColor(my_rand(px), my_rand(py), my_rand(px*py), (Uint8) (255 * (length/max)));
                float coeff = 0.4f;
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
            for (int i = 0; i < effect->particle_count; i++) {
                particle_tick(effect, i);

                Effect_Particle *particle = &effect->particles[i];

                int px = (int)particle->x;
                int py = (int)particle->y;

                int p2x = (int) (px - particle->vx*3);
                int p2y = (int) (py - particle->vy*3);

                RenderColor(255, 255, 255, 32);
                RenderLineRelative(target, px, py, p2x, p2y);
            }
            break;
        }
        default: break;
    }
}
