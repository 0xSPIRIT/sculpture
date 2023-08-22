#define BACKGROUND_PARTICLE_COUNT 256

typedef struct {
    SDL_Surface *surface;
    f64 time;
    
    vec2 *field;
    vec2 particles[BACKGROUND_PARTICLE_COUNT];
} Background;

static Background background_init(void);
static void background_setup_vector_field(vec2 *field, int w, int h);
static void background_setup_particles(vec2 *particles, int w, int h);
static void background_draw(int target, Background *bg);
