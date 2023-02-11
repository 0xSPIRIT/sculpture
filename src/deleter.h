#define DELETER_POINT_COUNT 100

struct Deleter {
    f32 x, y;
    int w, h;
    f64 angle;
    bool is_rotating;
    SDL_Texture *texture;
};