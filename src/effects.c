#include "effects.h"

#include <stdlib.h>
#include <SDL2/SDL.h>

#include "grid.h"
#include "globals.h"

#include "util.h"

struct Effect current_effect = {0};

void effect_set(int type) {
    current_effect.type = type;
    if (current_effect.particles) {
        free(current_effect.particles);
    }

    switch (type) {
    case EFFECT_NONE:
        current_effect.particles = NULL;
        return;
    case EFFECT_SNOW:
        current_effect.particle_count = 100;
        break;
    case EFFECT_RAIN:
        current_effect.particle_count = 50;
        break;
    }

    current_effect.particles = calloc(current_effect.particle_count, sizeof(struct Effect_Particle));

    switch (type) {
    case EFFECT_SNOW:
        for (int i = 0; i < current_effect.particle_count; i++) {
            struct Effect_Particle *particle = &current_effect.particles[i];
            particle->x = rand()%gw;
            particle->y = rand()%gh;

            particle->vx = 0.3 + randf(0.5);
            particle->vy = 0.3 + randf(1.0);

            const float spd = 0.3;
            particle->vx *= spd;
            particle->vy *= spd;
        }
        break;
    case EFFECT_RAIN:
        for (int i = 0; i < current_effect.particle_count; i++) {
            struct Effect_Particle *particle = &current_effect.particles[i];
            particle->x = rand()%gw;
            particle->y = rand()%gh;

            particle->vx = 0.3 + randf(0.3);
            particle->vy = 1.0;
        }
        break;
    }
}

void effect_tick(struct Effect *effect) {
    if ((paused && !step_one) || effect->type == EFFECT_NONE)
        return;
    
    switch (effect->type) {
    case EFFECT_SNOW: case EFFECT_RAIN:
        for (int i = 0; i < effect->particle_count; i++) {
            struct Effect_Particle *particle = &current_effect.particles[i];
            particle->x += particle->vx;
            particle->y += particle->vy;

            if (effect->type == EFFECT_RAIN) {
                particle->vx += 0.005 * sin(SDL_GetTicks() / 1000.0);

                if (is_in_bounds((int)particle->x, (int)particle->y) && grid[(int)particle->x + ((int)particle->y)*gw].type) {
                    particle->x = rand()%gw;
                    particle->y = 0;
                }
            }

            if (particle->x < 0) particle->x = gw-1;
            if (particle->y < 0) particle->y = gh-1;

            if (particle->x-particle->vx*3 > gw-1) particle->x = 0;
            if (particle->y-particle->vy*3 > gh-1) particle->y = 0;
        }
        break;
    }
}

void effect_draw(struct Effect *effect) {
    if (effect->type == EFFECT_NONE)
        return;
    
    switch (effect->type) {
    case EFFECT_SNOW:
        for (int i = 0; i < effect->particle_count; i++) {
            struct Effect_Particle *particle = &current_effect.particles[i];
            float length = sqrt(particle->vx*particle->vx + particle->vy*particle->vy);
            const float max = 1.125;

            int px = (int)particle->x;
            int py = (int)particle->y;

            if (grid[px+py*gw].type && (length/max) < 0.25) continue;

            SDL_SetRenderDrawColor(renderer, 255, 255, 255, (Uint8) (255 * (length/max)));
            SDL_RenderDrawPoint(renderer, px, py);
        }
        break;
    case EFFECT_RAIN:
        for (int i = 0; i < effect->particle_count; i++) {
            struct Effect_Particle *particle = &current_effect.particles[i];

            int px = (int)particle->x;
            int py = (int)particle->y;

            int p2x = px - particle->vx*3;
            int p2y = py - particle->vy*3;

            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 32);
            SDL_RenderDrawLine(renderer, px, py, p2x, p2y);
        }
        break;
    }
}
