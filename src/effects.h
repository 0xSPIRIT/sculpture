#define EFFECT_SCALE 3

typedef enum Effect_Type {
    EFFECT_NONE,
    EFFECT_SNOW,
    EFFECT_RAIN,
    EFFECT_WIND
} Effect_Type;

typedef struct Effect_Particle {
    f32 x, y;
    f32 vx, vy;
} Effect_Particle;

#define MAX_SPLASH 512
typedef struct {
    Effect_Particle splashes[MAX_SPLASH];
    int splash_count;
} Rain_Splash;

typedef struct Effect {
    Effect_Type type;
    Effect_Particle *particles;
    int particle_count;

    bool high_fidelity;

    SDL_Rect bounds;
    Rain_Splash rain;
} Effect;

static Cell_Type effect_picked_up(Effect *effect);
static void effect_handle_placer(Effect *effect, int x, int y, int r);
