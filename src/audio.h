// NOTE: Out of MIX_MAX_VOLUME (128)
#define AUDIO_CHISEL_VOLUME       (55)
#define AUDIO_GUI_VOLUME          (68)
#define AUDIO_MUSIC_VOLUME        (128)
#define AUDIO_AMBIENCE_VOLUME     (29)
#define AUDIO_TITLESCREEN_VOLUME  Volume(1.00)

#define AUDIO_PLAY_AMBIANCE 1
#define AUDIO_PLAY_MUSIC    1

#define Volume(x) ((int) ((f64)(x) * MIX_MAX_VOLUME))

enum {
    AUDIO_CHANNEL_NONE,
    AUDIO_CHANNEL_WOOSH,
    AUDIO_CHANNEL_CHISEL,
    AUDIO_CHANNEL_NARRATOR,
    AUDIO_CHANNEL_GUI,
    AUDIO_CHANNEL_AMBIENCE,
    AUDIO_CHANNEL_MUSIC,
    AUDIO_CHANNEL_COUNT
};

typedef enum {
    AMBIENCE_NONE, // no sound
    AMBIENCE_NORMAL,
    AMBIENCE_RAIN,
    AMBIENCE_RAIN_REVERSED
} AmbienceType;

typedef enum {
    MUSIC_NONE,
    MUSIC_FARCE,
    MUSIC_EXPLITIVE,
    MUSIC_PHOTOGRAPH
} MusicType;

typedef struct {
    AmbienceType ambience;
    int ambience_volume;

    MusicType music;
    int music_volume;
    bool music_end;

    // used for audio_lower_channel_for_ms
    f32 fader; // 0.0 to 1.0
    int old_volume;
    int time;
    bool fade_initted, waiting;
} Audio_Handler;

typedef struct {
    Mix_Chunk *sound;
    int volume; // out of 128 (MIX_MAX_VOLUME)
} Sound;

static void play_sound(int channel, Sound sound, int loops);

static void audio_halt_ambience(void);
static void audio_set_ambience_accordingly(void);
static void audio_set_ambience_levels(void);
static void audio_set_ambience(AmbienceType ambience);

static void audio_halt_music(void);
static void audio_set_music_accordingly(void);
static void audio_set_music(MusicType music);
