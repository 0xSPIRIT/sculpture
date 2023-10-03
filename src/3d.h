#ifdef ALASKA_RELEASE_MODE
    #define SCALE_3D 1
#else
    #define SCALE_3D 0.5
#endif

enum Object3D_State {
    OBJECT_ZOOM,
    OBJECT_ROTY,
    OBJECT_FALL,
    OBJECT_DONE
};

typedef struct {
    bool active;
    SDL_Texture *texture;
    int state;
    f32 y, z;
    f32 yrot;
    f32 xrot;
    f32 acc, vel, jerk;

    int t, hold;
    f32 t2;
    int timer;
} Object3D;

typedef struct {
    f32 x, y;
} vec2;
typedef struct {
    f32 x, y, z;
} vec3;

typedef struct {
    vec2 p;   // Position
    vec2 tex; // Texture coordinates (0 to 1)
} Vertex;
