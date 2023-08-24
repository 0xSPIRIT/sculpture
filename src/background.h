#define BACKGROUND_PARTICLE_COUNT 6

typedef struct {
    int value; // 0 = inactive
    f32 x, y;  // high precision position
    f32 sx, sy; // start position
    f32 timer;
} Background_Particle;

typedef struct {
    f32 sx, sy;
    f32 x, y, dir_x, dir_y;
    f32 length;
} Star;

typedef struct {
    SDL_Surface *surface;
    f32 time;
    
    Background_Particle particles[BACKGROUND_PARTICLE_COUNT];
    Star star;
} Background;

static Background background_init(void);
static void background_setup_star(Star *star);
static void background_setup_vector_field(vec2 *field, int w, int h, f32 time);
static void background_setup_particles(Background_Particle  *particles, int w, int h);
static void background_draw(int target, Background *bg);
