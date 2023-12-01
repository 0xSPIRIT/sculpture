enum Object3D_State {
    OBJECT_ZOOM,
    OBJECT_ROTY,
    OBJECT_FALL,
    OBJECT_DONE
};

typedef struct {
    bool active;
    int state;
    f32 y, z;
    f32 yrot;
    f32 xrot;
    f32 acc, vel, jerk;
    
    Texture texture; // The texture on the plane
    SDL_Surface *surf; // The surface version of teh texture.

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
