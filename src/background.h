typedef struct {
    SDL_Surface *surface;
    float time;
} Background;

Background background_init(void);
void background_draw(Background *background);