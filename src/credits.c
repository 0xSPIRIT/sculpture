static bool credits_screen(int target, Credits_Screen *screen, const char *lines[], int line_count);

static void credits_init_timers(Credits *c) {
    for (int i = 0; i < TIMERS_COUNT; i++) {
        c->timers[i] = -1;
    }
}

static bool credits_timer(int index, int timer_max) {
    Credits *c = &gs->credits;

    if (c->timers[index] == -2) return true;

    if (c->timers[index] == -1) { // -1 means unset & ready to do another timer
        c->timers[index] = timer_max;
    } else {
        c->timers[index]--;
        if (c->timers[index] <= 0) {
            c->timers[index] = -2; // -2 means unset & finished time.
            return true;
        }
    }

    return false;
}

static void credits_timer_reset(int index) {
    Credits *c = &gs->credits;
    c->timers[index] = -1;
}

static bool is_credits_timer_reset(int index) {
    Credits *c = &gs->credits;
    return (c->timers[index] == -1);
}


static void credits_run(int target) {
    Credits *c = &gs->credits;

    if (c->state == CREDITS_OFF) return;

    if (!c->initted) {
        credits_init_timers(c);
        c->initted = true;
    }

    switch (c->state) {
        case CREDITS_DELAY: {
            if (credits_timer(0, 4*60)) { // 4*60
                c->state = CREDITS_SHOW_1;
                credits_timer_reset(0);
            }
            break;
        }
        case CREDITS_SHOW_1: {
            const char *lines[] = { "Created by spiritwolf" };
            if (credits_screen(target, &c->s, lines, ArrayCount(lines))) {
                c->state = CREDITS_SHOW_2;
                memset(&c->s, 0, sizeof(Credits_Screen));
            }
            break;
        }
        case CREDITS_SHOW_2: {
            const char *lines[] = { "Background Art by:", "Joshwa with a W", "knightmere" };
            if (credits_screen(target, &c->s, lines, ArrayCount(lines))) {
                c->state = CREDITS_SHOW_3;
                memset(&c->s, 0, sizeof(Credits_Screen));
            }
            break;
        }
        case CREDITS_SHOW_3: {
            const char *lines[] = { "Thank you for playing." };
            if (credits_screen(target, &c->s, lines, ArrayCount(lines))) {
                c->state = CREDITS_END;
            }
            break;
        }
        case CREDITS_END: {
        } break;
    }
}

/////////////////////////////////

// `screen` should be simply a zeroed Credits_Screen struct on the first
// call to this function. It's used to keep state.
//
// This should be called every frame.
//
// Returns when that screen is done
static bool credits_screen(int target, Credits_Screen *screen, const char *lines[], int line_count) {
    // times in frames
    f64 time = 360;
    f64 hang_time = 60; // The hang-time after the text fades out.
    f64 fade_time = 50;

    if (screen->timer >= time) {
        screen->timer++;
        return (screen->timer >= time+hang_time);
    }

    int fade_state = FADE_FULL;
    if (screen->timer < 45) {
        fade_state = FADE_IN;
    } else if (screen->timer >= time-fade_time) {
        fade_state = FADE_OUT;
    }

    switch (fade_state) {
        case FADE_IN: {
            screen->fade += 1.0/fade_time;
        } break;
        case FADE_FULL: {
            screen->fade = 1;
        } break;
        case FADE_OUT: {
            screen->fade -= 1.0/fade_time;
        } break;
    }

    screen->timer++;

    f64 fade = 255 * (1 - screen->fade);
    SDL_Color col = { fade, fade, fade, 255 };

    for (int i = 0; i < line_count; i++) {
        Render_Text_Data text_data = {0};
        sprintf(text_data.identifier, "end%d", i);
        text_data.font = gs->fonts.font_times;
        strcpy(text_data.str, lines[i]);
        text_data.x = gs->game_width/2;
        text_data.y = gs->game_height/2 - Scale(100) + i * text_data.font->char_height;
        text_data.foreground = col;
        text_data.alignment = ALIGNMENT_CENTER;
        text_data.render_type = TEXT_RENDER_BLENDED;

        RenderText(target, &text_data);
    }

    return false;
}
