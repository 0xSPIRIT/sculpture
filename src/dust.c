static void dust_init(void) {
    if (!gs->dust) {
        gs->dust = PushArray(gs->persistent_memory, MAX_DUST_COUNT, sizeof(Dust));
    } else {
        memset(gs->dust, 0, MAX_DUST_COUNT*sizeof(Dust));
    }

    gs->dust_count = 0;
}

static void emit_dust(enum Cell_Type type,
                      int x,
                      int y,
                      f64 vx,
                      f64 vy,
                      bool from_destroy_tool)
{
    if (gs->dust_count >= MAX_DUST_COUNT) return;

    gs->dust[gs->dust_count++] = (Dust){
        .type = type,
        .x = x,
        .y = y,
        .vx = vx,
        .vy = vy,
        .timer = 0,
        .timer2 = 0,
        .timer_max = rand()%35+60,
        .going_into_inventory = false,
        .destroyed_via_tool = from_destroy_tool,
        .rand = my_rand((int)(x+y+vx+vy))
    };
}

static void emit_dust_explosion(enum Cell_Type type,
                                int x,
                                int y,
                                int count,
                                bool from_destroy_tool)
{
    for (int i = 0; i < count; i++) {
        f64 angle = randf(2*M_PI);
        f64 vx, vy;
        if (y == gs->gh-1) {
            vx = (rand()%2 == 0) ? 1 : -1;
            vy = -1;
        } else {
            vx = cos(angle);
            vy = sin(angle);
        }
        emit_dust(type, x, y, vx, vy, from_destroy_tool);
    }
}

static void dust_remove(int i) {
    gs->dust_count--;
    for (int j = i; j < gs->dust_count; j++) {
        gs->dust[j] = gs->dust[j+1];
    }
}

static SDL_Color dust_strobe_color(SDL_Color input) {
    SDL_Color result;

    int v = 30*sin(input.r + SDL_GetTicks()/100.0);

    result.r = 0.5 * clamp(input.r + v, 0, 255);
    result.g = 0.5 * clamp(input.g + v, 0, 255);
    result.b = 0.5 * clamp(input.b + v, 0, 255);
    result.a = 255;

    return result;
}

static void dust_grid_run(int target) {
    for (int i = gs->dust_count-1; i >= 0; i--) {
        Dust *dust = &gs->dust[i];

        if (!gs->paused) {
            if (distance(dust->x, dust->y, gs->gw/2, 0) < 3) {
                dust_remove(i);
                if (dust->destroyed_via_tool)
                    play_sound(AUDIO_CHANNEL_PING, gs->audio.ping, 0);
                continue;
            }

            if (dust->timer == dust->timer_max) {
                if (can_recipes_be_active()) {
                    dust->going_into_inventory = true;
                    dust->timer2 = 0;
                } else {
                    dust_remove(i);
                    continue;
                }
            }

            dust->timer++;

            if (dust->going_into_inventory) {
                f64 dx = gs->gw / 2 - dust->x;
                f64 dy = -dust->y;
                f64 length = sqrt(dx*dx + dy*dy);
                dx /= length;
                dy /= length;

                f64 max = 30;
                dust->timer2 = min(dust->timer2, max);
                f64 coeff = dust->timer2 / max;

                dust->timer2++;

                dust->vx = coeff * 2 * dx;
                dust->vy = coeff * 2 * dy;
            } else if (dust->y < gs->gw-1) {
                dust->vy += 0.2;
            }

            dust->x += dust->vx;
            dust->y += dust->vy;

            if (dust->y >= gs->gh) {
                dust->y = gs->gh-1;
                dust->vy *= -(randf(0.5)+0.25);
                dust->vx *= 0.75 + randf(0.25)-0.125;
            }

            if (fabs(dust->y-gs->gh-1) < 2 && fabs(dust->vy) < 1) {
                dust->y = gs->gh-1;
                dust->vy = 0;
                dust->vx *= 0.25;
            }
        }

        // Draw:
        u8 alpha = 200;

        SDL_Color c = pixel_from_index_grid(gs->grid,
                                            dust->type,
                                            0);
        c = dust_strobe_color(c);
        apply_lighting_to_color(&gs->lighting, &c, dust->x, dust->y);

        RenderColor(c.r, c.g, c.b, alpha);
        RenderPointRelative(target, dust->x, dust->y);
    }
}
