#define AUDIO_CHISEL_VOLUME       Volume(0.20)
#define AUDIO_GUI_VOLUME          Volume(0.27)
#define AUDIO_MUSIC_VOLUME        Volume(1.00)
#define AUDIO_TITLESCREEN_VOLUME  Volume(1.00)

#define AUDIO_PLAY_AMBIANCE 1

#define Volume(x) ((int) ((f64)(x) * MIX_MAX_VOLUME))

enum {
    AUDIO_CHANNEL_CHISEL,
    AUDIO_CHANNEL_NARRATOR,
    AUDIO_CHANNEL_GUI,
    AUDIO_CHANNEL_AMBIENCE,
    AUDIO_CHANNEL_MUSIC,
};

typedef enum {
    AMBIENCE_NONE, // no sound
    AMBIENCE_NORMAL,
    AMBIENCE_RAIN
} AmbienceType;

typedef enum {
    MUSIC_NONE,
    MUSIC_EXPLITIVE
} MusicType;

typedef struct {
    AmbienceType ambience;
    int ambience_volume;
    
    MusicType music;
    int music_volume;
} Audio_Handler;

typedef struct {
    Mix_Chunk *sound;
    int volume; // out of 128 (MIX_MAX_VOLUME)
} Sound;

void play_sound(int channel, Sound sound, int loops);

void audio_halt_ambience(void);
void audio_set_ambience_accordingly(void);
void audio_set_ambience_levels(void);
void audio_set_ambience(AmbienceType ambience);

void audio_halt_music(void);
void audio_set_music_accordingly(void);
void audio_set_music(MusicType music);