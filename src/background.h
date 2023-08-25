#define BACKGROUND_PARTICLE_COUNT 6

typedef struct {
    int value; // 0 = inactive
    f32 x, y;  // high precision position
    f32 sx, sy; // start position
    f32 timer;
} Background_Particle;

typedef struct {
    SDL_Surface *surface;
    f32 time;
    
    Background_Particle particles[BACKGROUND_PARTICLE_COUNT];
} Background;

static Background background_init(void);
static void background_draw(int target, Background *bg);
