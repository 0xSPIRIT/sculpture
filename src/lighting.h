#define MAX_LIGHTS 16

typedef struct {
    f64 x, y;
    f64 strength; // 0 to 1
    int radius;
    bool active;
} Light;

typedef struct {
    Light lights[MAX_LIGHTS];
    int light_count;

    Light *main_light, *chisel_light;
} Lighting;
