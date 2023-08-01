#define AUDIO_AMBIENCE_RAIN_VOLUME Volume(0.16)
#define AUDIO_AMBIENCE_VOLUME      Volume(0.40)
#define AUDIO_CHISEL_VOLUME        Volume(0.20)
#define AUDIO_GUI_VOLUME           Volume(0.27)
#define AUDIO_TITLESCREEN_VOLUME   Volume(1.00)

#define AUDIO_PLAY_AMBIANCE 1

#define Volume(x) ((int) ((f64)(x) * MIX_MAX_VOLUME))

enum {
    AUDIO_CHANNEL_CHISEL,
    AUDIO_CHANNEL_NARRATOR,
    AUDIO_CHANNEL_GUI,
    AUDIO_CHANNEL_MUSIC
};

typedef struct {
    int ambience; // Current ambience
    f64 ambience_volume, ambience_volume_to;
} Audio_Handler;

enum {
    AMBIENCE_NONE, // no sound
    AMBIENCE_NORMAL,
    AMBIENCE_RAIN
};

void audio_halt_music(void);
void audio_play_music(Mix_Chunk *music);
void audio_set_ambience_accordingly(void);
void audio_set_ambience_levels(void);
void audio_set_ambience(int ambience);
