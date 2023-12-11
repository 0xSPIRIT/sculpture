void channel_finished(int channel) {
    Audio_Handler *handler = &gs->audio_handler;
    if (channel == AUDIO_CHANNEL_MUSIC && handler->music) {
        audio_set_music(handler->music, false);
    }
}

static void audio_handler_init(void) {
    Mix_ChannelFinished(channel_finished);
}

// This is called every frame and sets the current music according to certain conditions.
static void audio_set_music_accordingly(void) {
#if !AUDIO_PLAY_MUSIC
    audio_set_music(MUSIC_NONE);
#else
    int level_number = gs->level_current+1;

    if (level_number <= 3) {
        audio_set_music(MUSIC_FRONTIER, true);
    } else if (level_number >= 4 && level_number <= 6) {
        audio_set_music(MUSIC_ABCS, true);
    } else if (level_number >= 8 && level_number <= 10) {
        audio_set_music(MUSIC_PHOTOGRAPH, true);
    } else if (level_number == 7 && gs->overlay.changes.music_started) {
        if (gs->level_completed) {
            audio_set_music(MUSIC_NONE, true);
        } else {
            audio_set_music(MUSIC_WEIRD, true);
        }
    } else if (gs->obj.active) {
        audio_set_music(MUSIC_EXPLITIVE, true);
    } else {
        audio_set_music(MUSIC_NONE, true);
    }
#endif
}

// Called every frame.
static void audio_set_ambience_accordingly(void) {
    Level *level = &gs->levels[gs->level_current];

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
    Mix_HaltChannel(AUDIO_CHANNEL_MUSIC);
    gs->audio_handler.music = 0;
    gs->audio_handler.music_volume = 0;
    gs->audio_handler.music_end = true;
}

static void audio_set_music(MusicType music, bool fade) {
    Audio_Handler *handler = &gs->audio_handler;

    if (!fade || handler->music == 0) {
        handler->music = music;
        switch (music) {
            case MUSIC_EXPLITIVE: {
                play_sound(AUDIO_CHANNEL_MUSIC, gs->audio.music0, -1);
            } break;
            case MUSIC_PHOTOGRAPH: {
                play_sound(AUDIO_CHANNEL_MUSIC, gs->audio.music1, -1);
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
    } else if (handler->music != music) {
        Mix_FadeOutChannel(AUDIO_CHANNEL_MUSIC, 1000);
        handler->music = music;
    }
}