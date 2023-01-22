#define EFFECT_SCALE 3

enum Effect_Type {
    EFFECT_NONE,
    EFFECT_SNOW,
    EFFECT_RAIN
};

struct Effect_Particle {
    f32 x, y;
    f32 vx, vy;
};

struct Effect {
    enum Effect_Type type;
    struct Effect_Particle *particles;
    int particle_count;
    
    int w, h;
};
