enum Credits_State {
    CREDITS_OFF,
    CREDITS_DELAY,
    CREDITS_SHOW,
};

typedef struct Credits {
    int state;
    int timer;
} Credits;