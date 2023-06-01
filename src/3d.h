#ifdef ALASKA_RELEASE_MODE
    //#define SCALE_3D 1
    #define SCALE_3D 0.5
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
    f64 y, z;
    f64 yrot;
    f64 xrot;
    f64 acc, vel, jerk;
    
    int t, hold;
    f64 t2;
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
    //vec3 col; // Color data per vertex. (0 to 255)
    vec2 tex; // GetTexture coordinates (0 to 1)
} Vertex;