#define FADE_T 0.04
#define FADE_EPSILON 5

enum {
    FADE_null,
    FADE_NARRATOR,
    FADE_LEVEL_INTRO,
    FADE_LEVEL_PLAY_IN,
    FADE_LEVEL_PLAY_OUT,
    FADE_LEVEL_FINISH,
    FADE_LEVEL_NARRATION,
    FADE_TITLESCREEN,
    FADE_TITLESCREEN_2,
    FADE_TITLESCREEN_3
};

typedef struct Fade {
    int id;
    bool active;
    f64 alpha, start_alpha, desired_alpha, time;
    f64 dt;
} Fade;
