enum {
    HAMMER_STATE_IDLE,
    HAMMER_STATE_WINDUP,
    HAMMER_STATE_ATTACK,
    HAMMER_STATE_BLOWBACK
};

typedef struct Hammer {
    int state;

    int dir;
    bool flip;

    bool dont_chisel_this_frame; // something you can set

    int x, y;
    f64 base_angle, angle, temp_angle;
    Texture *tex;

    f64 t;
} Hammer;
