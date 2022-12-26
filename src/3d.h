#define SCALE_3D 0.2

enum Object3D_State {
    OBJECT_ZOOM,
    OBJECT_ROTY,
    OBJECT_FALL,
    OBJECT_DONE
};

struct Object3D {
    SDL_Texture *texture;
    int state;
    f64 y, z;
    f64 yrot;
    f64 xrot;
    f64 acc, vel, jerk;
};

typedef struct vec2 {
    f32 x, y;
} vec2;
typedef struct vec3 {
    f32 x, y, z;
} vec3;

typedef struct Vertex {
    vec2 p; // Position
    vec3 col; // Color data per vertex.
} Vertex;