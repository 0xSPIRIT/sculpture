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

typedef struct {
    f64 x, y;
} vec2;
typedef struct {
    f64 x, y, z;
} vec3;