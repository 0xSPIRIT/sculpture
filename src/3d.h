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

struct Object3D {
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
};

typedef struct vec2 {
    f32 x, y;
} vec2;
typedef struct vec3 {
    f32 x, y, z;
} vec3;

typedef struct Vertex {
    vec2 p;   // Position
    //vec3 col; // Color data per vertex. (0 to 255)
    vec2 tex; // Texture coordinates (0 to 1)
} Vertex;