#define FADE_T 0.04
#define FADE_EPSILON 5

enum {
    FADE_NULL,
    FADE_NARRATOR,
    FADE_LEVEL_INTRO,
    FADE_LEVEL_PLAY_IN,
    FADE_LEVEL_PLAY_OUT,
    FADE_LEVEL_FINISH,
    FADE_LEVEL_NARRATION,
};

typedef struct Fade {
    int id;
    bool active;
    f64 alpha, start_alpha, desired_alpha, time;
    f64 dt;
} Fade;