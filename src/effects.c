void effect_set(int type) {
    gs->current_effect.type = type;

    switch (type) {
    case EFFECT_NONE:
        gs->current_effect.particles = NULL;
        return;
    case EFFECT_SNOW:
        gs->current_effect.particle_count = 100;
        break;
    case EFFECT_RAIN:
        gs->current_effect.particle_count = 50;
        break;
    }

    if (gs->current_effect.particles == NULL) {
        gs->current_effect.particles = arena_alloc(gs->persistent_memory, gs->current_effect.particle_count, sizeof(struct Effect_Particle));
    }

    switch (type) {
    case EFFECT_SNOW:
        for (int i = 0; i < gs->current_effect.particle_count; i++) {
            struct Effect_Particle *particle = &gs->current_effect.particles[i];
            particle->x = (f32) (rand()%gs->gw);
            particle->y = (f32) (rand()%gs->gh);

            particle->vx = 0.3f + randf(0.5);
            particle->vy = 0.3f + randf(1.0);

            const f32 spd = 0.3f;
            particle->vx *= spd;
            particle->vy *= spd;
        }
        break;
    case EFFECT_RAIN:
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

void effect_tick(struct Effect *effect) {
    if ((gs->paused && !gs->step_one) || effect->type == EFFECT_NONE)
        return;
    
    switch (effect->type) {
    case EFFECT_SNOW: case EFFECT_RAIN:
        for (int i = 0; i < effect->particle_count; i++) {
            struct Effect_Particle *particle = &gs->current_effect.particles[i];
            particle->x += particle->vx;
            particle->y += particle->vy;

            if (effect->type == EFFECT_RAIN) {
                particle->vx += 0.003f * sinf(SDL_GetTicks() / 1000.f);
            }

            if (is_in_bounds((int)particle->x, (int)particle->y) && gs->grid[(int)particle->x + ((int)particle->y)*gs->gw].type) {
                particle->x = (f32) (rand()%gs->gw);
                particle->y = 0;
            }

            if (particle->x < 0) particle->x = (f32) gs->gw-1;
            if (particle->y < 0) particle->y = (f32) gs->gh-1;

            if (particle->x-particle->vx*3 > gs->gw-1) particle->x = 0;
            if (particle->y-particle->vy*3 > gs->gh-1) particle->y = 0;
        }
        break;
    default:
        break;
    }
}

void effect_draw(struct Effect *effect) {
    if (effect->type == EFFECT_NONE)
        return;
    
    switch (effect->type) {
    case EFFECT_SNOW:
        for (int i = 0; i < effect->particle_count; i++) {
            struct Effect_Particle *particle = &gs->current_effect.particles[i];
            f32 length = (f32) sqrt(particle->vx*particle->vx + particle->vy*particle->vy);
            const f32 max = 1.125;

            int px = (int)particle->x;
            int py = (int)particle->y;

            // Confetti Mode:
            const bool confetti_mode = false;

            if (confetti_mode)
                SDL_SetRenderDrawColor(gs->renderer, my_rand(px), my_rand(py), my_rand(px*py), (Uint8) (255 * (length/max)));
            else
                SDL_SetRenderDrawColor(gs->renderer, 255, 255, 255, (Uint8) (255 * (length/max)));

            SDL_RenderDrawPoint(gs->renderer, px, py);
        }
        break;
    case EFFECT_RAIN:
        for (int i = 0; i < effect->particle_count; i++) {
            struct Effect_Particle *particle = &gs->current_effect.particles[i];

            int px = (int)particle->x;
            int py = (int)particle->y;

            int p2x = (int) (px - particle->vx*3);
            int p2y = (int) (py - particle->vy*3);

            SDL_SetRenderDrawColor(gs->renderer, 255, 255, 255, 32);
            SDL_RenderDrawLine(gs->renderer, px, py, p2x, p2y);
        }
        break;
    default:
        break;
    }
}
