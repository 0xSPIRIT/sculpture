typedef struct {
    SDL_Surface *surface;
    f32 time;
} Background;

static Background background_init(void);
static void background_draw(int target, Background *bg);
