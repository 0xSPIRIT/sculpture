#define EFFECT_SCALE 3

enum Effect_Type {
    EFFECT_NONE,
    EFFECT_SNOW,
    EFFECT_RAIN
};

typedef struct Effect_Particle {
    f32 x, y;
    f32 vx, vy;
} Effect_Particle;

typedef struct Effect {
    enum Effect_Type type;
    Effect_Particle *particles;
    int particle_count;
    
    int w, h;
} Effect;
