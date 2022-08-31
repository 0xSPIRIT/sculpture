#ifndef EFFECTS_H_
#define EFFECTS_H_ 

enum Effect_Type {
    EFFECT_NONE,
    EFFECT_SNOW,
    EFFECT_RAIN
};

struct Effect_Particle {
    float x, y;
    float vx, vy;
};

struct Effect {
    int type;
    struct Effect_Particle *particles;
    int particle_count;
};

extern struct Effect current_effect;

void effect_set(int type);
void effect_tick(struct Effect *effect);
void effect_draw(struct Effect *effect);

#endif // EFFECTS_H_
