#define EFFECT_SCALE 3

typedef enum Effect_Type {
    EFFECT_NONE,
    EFFECT_SNOW,
    EFFECT_RAIN
} Effect_Type;

typedef struct Effect_Particle {
    f32 x, y;
    f32 vx, vy;
} Effect_Particle;

typedef struct Effect {
    Effect_Type type;
    Effect_Particle *particles;
    int particle_count;

    bool high_fidelity;

    SDL_Rect bounds;
} Effect;
