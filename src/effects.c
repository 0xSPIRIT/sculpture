static void effect_reset_snow(bool high_fidelity) {
    for (int i = 0; i < gs->current_effect.particle_count; i++) {
        Effect_Particle *particle = &gs->current_effect.particles[i];

        SDL_Rect bounds = gs->current_effect.bounds;

        particle->x = (f32) (bounds.x+(rand()%bounds.w));
        particle->y = (f32) (bounds.y+(rand()%bounds.h));

        if (!high_fidelity) {
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
        if (high_fidelity) {
            spd = EFFECT_SCALE;
        }

        particle->vx *= spd;
        particle->vy *= spd;
    }
}

static void effect_set(int type, bool high_fidelity, int x, int y, int w, int h) {
    Effect *effect = &gs->current_effect;

    effect->type = type;
    effect->high_fidelity = high_fidelity;

    effect->bounds = (SDL_Rect){x, y, w, h};

    switch (type) {
        case EFFECT_NONE: {
            effect->particles = NULL;
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
            effect->particle_count = 50;
            break;
        }
    }

    if (effect->particles == NULL) {
        effect->particles = PushArray(gs->persistent_memory, effect->particle_count, sizeof(Effect_Particle));
    }

    switch (type) {
        case EFFECT_SNOW: {
            effect_reset_snow(high_fidelity);
            break;
        }
        case EFFECT_RAIN: {
            for (int i = 0; i < effect->particle_count; i++) {
                Effect_Particle *particle = &effect->particles[i];
                particle->x = (f32) (rand()%gs->gw);
                particle->y = (f32) (rand()%gs->gh);

                particle->vx = 0.3f + randf(0.3f);
                particle->vy = 1.f;
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
            Effect_Particle *particle = &gs->current_effect.particles[i];

            int reverse = (gs->level_current+1 >= 8) ? -1 : 1;

            particle->x += reverse * particle->vx;
            particle->y += reverse * particle->vy;

            if (effect->type == EFFECT_RAIN) {
                particle->vx += 0.003f * sinf(SDL_GetTicks() / 1000.f);
            }

            if (particle->x < effect->bounds.x) particle->x = (f32) effect->bounds.w-1;
            if (particle->y < effect->bounds.y) particle->y = (f32) effect->bounds.h-1;

            if (particle->x-particle->vx*3 > effect->bounds.w-1) particle->x = effect->bounds.x;
            if (particle->y-particle->vy*3 > effect->bounds.h-1) particle->y = effect->bounds.y;

            break;
        }
        default: break;
    }
}

static void effect_draw(int target, Effect *effect, bool draw_points, int only_slow) {
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

                if (!effect->high_fidelity) {
                    if (particle->x < gs->gw/2 + gs->render.view.x/gs->S) continue;
                    if (particle->y < gs->gw/2 + gs->render.view.y/gs->S) continue;
                    if (particle->x > gs->gw/2 + gs->render.view.x/gs->S+64) continue;
                    if (particle->y > gs->gw/2 + gs->render.view.y/gs->S+64) continue;
                } else {
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
                if (effect->high_fidelity) {
                    RenderColor(col, col, col, 255);
                } else {
                    RenderColor(255, 255, 255, col);
                }

                if (draw_points) {
                    RenderPoint(target, px, py);
                } else {

                    if (gs->levels[gs->level_current].state == LEVEL_STATE_NARRATION) {
                        RenderFillCircle(target,
                                         gs->window_width * px/effect->bounds.w,
                                         gs->window_height * py/effect->bounds.h,
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

                RenderColor(255, 255, 255, 32);
                SDL_RenderDrawLine(gs->renderer, px, py, p2x, p2y);
            }
            break;
        }
        default: break;
    }
}
