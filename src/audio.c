// This is called every frame and sets the current music according to certain conditions.
void audio_set_music_accordingly(void) {
    int level_number = gs->level_current+1;
    
    if (level_number == 11 && gs->obj.active) {
        audio_set_music(MUSIC_EXPLITIVE);
    } else {
        audio_set_music(MUSIC_NONE);
    }
}

// Called every frame.
void audio_set_ambience_accordingly(void) {
    Level *level = &gs->levels[gs->level_current];

    if (level->state == LEVEL_STATE_INTRO) {
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
    Mix_PlayChannel(channel, sound.sound, loops);
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
}

void audio_set_music(MusicType music) {
    Audio_Handler *handler = &gs->audio_handler;
    
    if (handler->music != music) {
        switch (music) {
            case MUSIC_EXPLITIVE: {
                play_sound(AUDIO_CHANNEL_MUSIC, gs->audio.music0, -1);
            } break;
            default: {
                audio_halt_music();
            } break;
        }
    
        handler->music = music;
    }
}