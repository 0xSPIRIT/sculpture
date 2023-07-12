#define AUDIO_AMBIENCE_RAIN_VOLUME Volume(0.12)
#define AUDIO_AMBIENCE_VOLUME Volume(0.30)
#define AUDIO_CHISEL_VOLUME   Volume(0.25)
#define AUDIO_GUI_VOLUME      Volume(0.20)
#define AUDIO_TITLESCREEN_VOLUME  Volume(0.75)

#define AUDIO_PLAY_AMBIANCE 1

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
void audio_play_music(Mix_Music *music);
void audio_set_ambience_accordingly(void);
void audio_set_ambience_levels(void);
void audio_set_ambience(int ambience);