#define SCALE_3D 1
#define SNOW_3D_PARTICLE_COUNT 250

typedef struct vec2 {
    f32 x, y;
} vec2;
typedef struct vec3 {
    f32 x, y, z;
} vec3;

typedef struct Vertex {
    vec2 p;   // Position
    vec3 col; // Color data per vertex. (0 to 255)
    vec2 tex; // Texture coordinates (0 to 1)
} Vertex;

enum Object3D_State {
    OBJECT_ZOOM,
    OBJECT_ROTY,
    OBJECT_FALL,
    OBJECT_DONE
};

struct Snow3D {
    vec3 p[SNOW_3D_PARTICLE_COUNT]; // Positions
    vec3 v[SNOW_3D_PARTICLE_COUNT]; // Velocities
};

struct Object3D {
    bool active;
    SDL_Surface *surf;
    int state;
    f32 y, z;
    f32 yrot;
    f32 xrot;
    f32 acc, vel, jerk;
};