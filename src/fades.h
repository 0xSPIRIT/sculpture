#define FADE_T 0.05
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

struct Fade {
    int id;
    bool active;
    float alpha, desired_alpha;
};