typedef struct {
    SDL_Surface *surface;
    f32 time;
} Background;

typedef struct {
    f32 x, y;
} Intro_Background;

static Background background_init(void);
static void background_draw(int target, Background *bg, int xoff, int yoff);
static void background_intro_init(Intro_Background *bg);
static void background_intro_draw(int target, Intro_Background *bg);
