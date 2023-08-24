// This is called every frame and sets the current music according to certain conditions.
void audio_set_music_accordingly(void) {
#if AUDIO_PLAY_MUSIC == 0
    audio_set_music(MUSIC_NONE);
#else
    int level_number = gs->level_current+1;
    
    if (gs->audio_handler.music_end) {
        audio_set_music(MUSIC_NONE);
        return;
    }
    
    if (level_number <= 3) {
        audio_set_music(MUSIC_FARCE);
    } else if (gs->obj.active) {
        audio_set_music(MUSIC_EXPLITIVE);
    } else {
        audio_set_music(MUSIC_NONE);
    }

    if (gs->audio_handler.lower_music) {
        if (audio_lower_channel_for(AUDIO_CHANNEL_MUSIC, 120)) {
            gs->audio_handler.lower_music = false;
        }
    }
#endif
}

// Called every frame.
void audio_set_ambience_accordingly(void) {
    Level *level = &gs->levels[gs->level_current];

    if (level->state == LEVEL_STATE_INTRO || gs->obj.active) {
        audio_set_ambience(0);
        return;
    }

    if (level->state == LEVEL_STATE_NARRATION || level->effect_type == EFFECT_SNOW || level->effect_type == EFFECT_NONE) {
        audio_set_ambience(AMBIENCE_NORMAL);
        return;
    }

    if (level->effect_type == EFFECT_RAIN) {
        audio_set_ambience(AMBIENCE_RAIN);
    }
}

void play_sound(int channel, Sound sound, int loops) {
    printf("Playing sound %p", sound.sound);
    Mix_PlayChannel(channel, sound.sound, loops);
}

// Must be called every frame.
// Returns when it's done.
bool audio_lower_channel_for(int channel, int frames) {
    Audio_Handler *handler = &gs->audio_handler;

    if (!handler->fade_initted) {
        handler->fader = 1;
        handler->time = gs->frames;
        handler->fader = 0.0;
        handler->waiting = true;
        handler->fade_initted = true;

        handler->old_volume = Mix_Volume(channel, (int)(handler->fader * MIX_MAX_VOLUME));
    }

    if (handler->waiting) {
        if (gs->frames - handler->time >= frames) {
            Mix_Volume(channel, handler->old_volume);

            // reset
            handler->fader = 0;
            handler->time = 0;
            handler->waiting = false;
            handler->fade_initted = false;
            handler->old_volume = 0;
            return true;
        }
    }

    return false;
}

void audio_lower_music_for_a_bit(void) {
    gs->audio_handler.lower_music = true;
}

void audio_halt_ambience(void) {
    Mix_HaltChannel(AUDIO_CHANNEL_AMBIENCE);
    gs->audio_handler.ambience = 0;
    gs->audio_handler.ambience_volume = 0;
}

// Should be called every frame.
void audio_set_ambience(AmbienceType ambience) {
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
        }
        gs->audio_handler.ambience = ambience;
    }

    audio_set_ambience_levels();
}

void audio_set_ambience_levels(void) {
    int volume = MIX_MAX_VOLUME;

    int level_state = gs->levels[gs->level_current].state;
    if (gs->gui.popup || level_state == LEVEL_STATE_OUTRO) {
        volume /= 2;
    }

    gs->audio_handler.ambience_volume = interpolate(gs->audio_handler.ambience_volume, volume, 5);
    Mix_Volume(AUDIO_CHANNEL_AMBIENCE, gs->audio_handler.ambience_volume);
}

void audio_halt_music(void) {
    Mix_HaltChannel(AUDIO_CHANNEL_MUSIC);
    gs->audio_handler.music = 0;
    gs->audio_handler.music_volume = 0;
    gs->audio_handler.music_end = true;
}

void audio_set_music(MusicType music) {
    Audio_Handler *handler = &gs->audio_handler;

    if (handler->music != music) {
        switch (music) {
            case MUSIC_EXPLITIVE: {
                play_sound(AUDIO_CHANNEL_MUSIC, gs->audio.music0, -1);
            } break;
            case MUSIC_FARCE: {
                play_sound(AUDIO_CHANNEL_MUSIC, gs->audio.music2, -1);
            } break;
            default: {
                audio_halt_music();
            } break;
        }

        handler->music = music;
    }
}
