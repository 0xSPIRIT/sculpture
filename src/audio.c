void audio_halt_music(void) {
    Mix_HaltChannel(AUDIO_CHANNEL_MUSIC);
    gs->audio_handler.ambience = 0;
    gs->audio_handler.ambience_volume = 0;
    gs->audio_handler.ambience_volume_to = 0;
}

void audio_play_music(Mix_Chunk *music) {
    Mix_PlayChannel(AUDIO_CHANNEL_MUSIC, music, -1);
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

// Should be called every frame.
void audio_set_ambience(int ambience) {
    if (gs->audio_handler.ambience != ambience) {
        switch (ambience) {
            case AMBIENCE_NONE: {
                audio_halt_music();
            } break;
            case AMBIENCE_NORMAL: {
                audio_play_music(gs->audio.ambience1);
            } break;
            case AMBIENCE_RAIN: {
                audio_play_music(gs->audio.music_rain);
            } break;
        }
        gs->audio_handler.ambience = ambience;
    }

    audio_set_ambience_levels();
}

void audio_set_ambience_levels(void) {
    f64 volume = 0;

    switch (gs->audio_handler.ambience) {
        case AMBIENCE_NONE: { } break;
        case AMBIENCE_NORMAL: {
            volume = AUDIO_AMBIENCE_VOLUME;
        } break;
        case AMBIENCE_RAIN: {
            volume = AUDIO_AMBIENCE_RAIN_VOLUME;
        } break;
    }

    int level_state = gs->levels[gs->level_current].state;
    if (gs->gui.popup || level_state == LEVEL_STATE_OUTRO) {
        volume /= 2;
    }

    gs->audio_handler.ambience_volume_to = volume;

    gs->audio_handler.ambience_volume = interpolate(gs->audio_handler.ambience_volume,
                                                    gs->audio_handler.ambience_volume_to,
                                                    0.5);
    Mix_Volume(AUDIO_CHANNEL_MUSIC, gs->audio_handler.ambience_volume);
}
