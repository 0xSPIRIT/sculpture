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
    
    int x, y, w, h;
    f64 base_angle, angle, temp_angle;
    SDL_Texture *tex;
    
    f64 t;
} Hammer;