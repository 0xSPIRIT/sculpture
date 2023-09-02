#define TIMERS_COUNT 10

enum Credits_State {
    CREDITS_OFF,
    CREDITS_DELAY,
    CREDITS_SHOW_1,
    CREDITS_SHOW_2,
    CREDITS_SHOW_3,
    CREDITS_END
};

enum Fade_State {
    FADE_IN,
    FADE_FULL,
    FADE_OUT
};

typedef struct Credits_Screen {
    f64 timer;
    f64 fade;
} Credits_Screen;

typedef struct Credits {
    bool initted;
    
    int state;
    int timers[TIMERS_COUNT];
    bool fade_out;
    
    Credits_Screen s;
} Credits;