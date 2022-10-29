#define DELETER_POINT_COUNT 10

struct Deleter {
    f32 x, y;
    int w, h;
    SDL_Texture *texture;

    Uint32 *pixels;

    bool active, was_active;

    int *highlights;
    int highlight_count;

    SDL_Point points[DELETER_POINT_COUNT];
    int point_count;

    int cooldown;
};