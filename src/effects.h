#define EFFECT_SCALE 3

enum Effect_Type {
    EFFECT_NONE,
    EFFECT_SNOW,
    EFFECT_RAIN
};

enum Only_Slow {
    ONLY_SLOW_ALL,
    ONLY_SLOW_SLOW,
    ONLY_SLOW_FAST,
};

typedef struct Effect_Particle {
    f32 x, y;
    f32 vx, vy;
} Effect_Particle;

typedef struct Effect {
    enum Effect_Type type;
    Effect_Particle *particles;
    int particle_count;

    bool high_fidelity;

    SDL_Rect bounds;
} Effect;
