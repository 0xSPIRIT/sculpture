// NOTE: Kind of a weird place to put this but it's the end of the project
//       so I don't care about formatting it in a proper place.
//       This is related to audio, and setting the volume of each of the
//       channels every frame.
static void assign_channel_volumes(Pause_Menu *menu, Audio_Handler *handler) {
    f32 master = menu->slider;
    Mix_Volume(AUDIO_CHANNEL_CHISEL,   master * handler->channel_volumes[AUDIO_CHANNEL_CHISEL]);
    Mix_Volume(AUDIO_CHANNEL_GUI,      master * handler->channel_volumes[AUDIO_CHANNEL_GUI]);
    Mix_Volume(AUDIO_CHANNEL_MUSIC,    master * handler->channel_volumes[AUDIO_CHANNEL_MUSIC]);
    Mix_Volume(AUDIO_CHANNEL_AMBIENCE, master * handler->channel_volumes[AUDIO_CHANNEL_AMBIENCE]);
    Mix_Volume(AUDIO_CHANNEL_MISC, master * MIX_MAX_VOLUME);
}

static void textures_load_backgrounds(Textures *texs, bool first_time) {
    if (!first_time) {
        for (int i = 0; i < 4; i++) {
            RenderDestroyTexture(&texs->texs[TEXTURE_BG_0+i]);
        }
    }

    texs->texs[TEXTURE_BG_0] = RenderLoadTexture("bg_0.png");
    texs->texs[TEXTURE_BG_1] = RenderLoadTexture("bg_1.png");
    texs->texs[TEXTURE_BG_2] = RenderLoadTexture("bg_2.png");
    texs->texs[TEXTURE_BG_3] = RenderLoadTexture("bg_3.png");
}