#define BACKGROUND_PARTICLE_COUNT 6

typedef struct {
    SDL_Surface *surface;
    f32 time;
} Background;

static Background background_init(void);
static void background_draw(int target, Background *bg, int xoff, int yoff);
