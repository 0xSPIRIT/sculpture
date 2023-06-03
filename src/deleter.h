#define DELETER_POINT_COUNT 100

typedef struct Deleter {
    f32 x, y;
    int w, h;
    f64 angle;
    bool is_rotating;
    Texture *texture;
} Deleter;