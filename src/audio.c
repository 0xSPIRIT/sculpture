void channel_finished(int channel) {
    Audio_Handler *handler = &gs->audio_handler;
    if (channel == AUDIO_CHANNEL_MUSIC) {
        if (handler->queued_music) {
            audio_set_music(handler->queued_music, false);
        } else {
            handler->music = 0;
        }
    }
}

static void audio_handler_init(void) {
    Mix_ChannelFinished(channel_finished);
}

static MusicType audio_get_music_for_level(int level_number) {
    if (level_number <= 3) return MUSIC_FRONTIER;
    if (level_number >= 4 && level_number <= 6) return MUSIC_ABCS;
    if (level_number >= 8 && level_number <= 10) return MUSIC_PHOTOGRAPH;
    return MUSIC_NONE;
}

static void audio_set_music_accordingly(void) {
#if !AUDIO_PLAY_MUSIC
    audio_set_music(MUSIC_NONE, false);
#else
    Audio_Handler *handler = &gs->audio_handler;

    int level_no = gs->level_current+1;
    MusicType music = audio_get_music_for_level(level_no);

    if (!handler->first) {
        handler->first = true;

        if (music == MUSIC_PHOTOGRAPH && music == audio_get_music_for_level(gs->level_previous+1)) {
        } else {
            handler->photograph_timer = 0;
            audio_set_music(music, true); // queues the music for us
        }
    } else {
        if (level_no == 7 && gs->overlay.changes.music_started) {
            if (gs->level_completed) {
                audio_set_music(MUSIC_NONE, false);
            } else {
                audio_set_music(MUSIC_WEIRD, true);
            }
        } else if (gs->obj.active && !handler->object_started) {
            audio_set_music(MUSIC_COMING_HOME, true);
            handler->object_started = true;
        } else if (music == MUSIC_PHOTOGRAPH) {
            handler->photograph_timer += 1.0/60.0;
            if (handler->photograph_timer >= 5*60) {
                audio_set_music(MUSIC_PHOTOGRAPH, true);
                handler->photograph_timer = 0;
            }
        }
    }
#endif
}

// Called every frame.
static void audio_set_ambience_accordingly(void) {
    Level *level = &gs->levels[gs->level_current];

#ifndef ALASKA_RELEASE_MODE
    audio_handler_init(); // Because of the DLL reloading the hooks don't point to the correct spot sometimes
#endif

    if (level->state == LEVEL_STATE_INTRO || gs->obj.active) {
        audio_set_ambience(0);
        return;
    }

    if (level->state == LEVEL_STATE_NARRATION ||
        level->effect_type == EFFECT_SNOW ||
        level->effect_type == EFFECT_NONE)
    {
        audio_set_ambience(AMBIENCE_NORMAL);
        return;
    }

    if (level->effect_type == EFFECT_WIND) {
        audio_set_ambience(AMBIENCE_WIND);
        return;
    }

    if (level->effect_type == EFFECT_RAIN) {
        if (gs->level_current+1 == 10) {
            audio_set_ambience(AMBIENCE_RAIN_REVERSED);
        } else {
            audio_set_ambience(AMBIENCE_RAIN);
        }
    }
}

static void play_sound(int channel, Sound sound, int loops) {
    Mix_PlayChannel(channel, sound.sound, loops);
}

static void audio_halt_ambience(void) {
    Mix_HaltChannel(AUDIO_CHANNEL_AMBIENCE);
    gs->audio_handler.ambience = 0;
    gs->audio_handler.ambience_volume = 0;
}

// Should be called every frame.
static void audio_set_ambience(AmbienceType ambience) {
    if (gs->audio_handler.ambience != ambience) {
        switch (ambience) {
            case AMBIENCE_NONE: {
                audio_halt_ambience();
            } break;
            case AMBIENCE_NORMAL: {
                play_sound(AUDIO_CHANNEL_AMBIENCE, gs->audio.ambience1, -1);
            } break;
            case AMBIENCE_RAIN: {
                play_sound(AUDIO_CHANNEL_AMBIENCE, gs->audio.ambience_rain, -1);
            } break;
            case AMBIENCE_WIND: {
                play_sound(AUDIO_CHANNEL_AMBIENCE, gs->audio.ambience_wind, -1);
            } break;
            case AMBIENCE_RAIN_REVERSED: {
                play_sound(AUDIO_CHANNEL_AMBIENCE, gs->audio.ambience_rain_reversed, -1);
            } break;
        }
        gs->audio_handler.ambience = ambience;
    }

    audio_set_ambience_levels();
}

static void audio_set_ambience_levels(void) {
    int volume = AUDIO_AMBIENCE_VOLUME;

    int level_state = gs->levels[gs->level_current].state;
    if (gs->gui.popup || level_state == LEVEL_STATE_OUTRO) {
        volume /= 2;
    }

    gs->audio_handler.ambience_volume = interpolate(gs->audio_handler.ambience_volume, volume, 1);
    gs->audio_handler.channel_volumes[AUDIO_CHANNEL_AMBIENCE] = gs->audio_handler.ambience_volume;
}

static void audio_halt_music(void) {
    gs->audio_handler.music = 0;
    gs->audio_handler.queued_music = 0;
    Mix_HaltChannel(AUDIO_CHANNEL_MUSIC);
}

static void audio_set_music(MusicType music, bool fade) {
    Audio_Handler *handler = &gs->audio_handler;

    if (fade && handler->music != music && handler->music != 0) {
        Mix_FadeOutChannel(AUDIO_CHANNEL_MUSIC, 1000);
        handler->queued_music = music;
        return;
    }

    if (!fade || handler->music == 0) {
        handler->music = music;
        handler->queued_music = 0;

        switch (music) {
            case MUSIC_COMING_HOME: {
                play_sound(AUDIO_CHANNEL_MUSIC, gs->audio.music0, 0);
            } break;
            case MUSIC_PHOTOGRAPH: {
                play_sound(AUDIO_CHANNEL_MUSIC, gs->audio.music1, 0);
            } break;
            case MUSIC_FRONTIER: {
                play_sound(AUDIO_CHANNEL_MUSIC, gs->audio.music2, -1);
            } break;
            case MUSIC_WEIRD: {
                play_sound(AUDIO_CHANNEL_MUSIC, gs->audio.music3, -1);
            } break;
            case MUSIC_ABCS: {
                play_sound(AUDIO_CHANNEL_MUSIC, gs->audio.music4, -1);
            } break;
            default: {
                audio_halt_music();
            } break;
        }
    }
}