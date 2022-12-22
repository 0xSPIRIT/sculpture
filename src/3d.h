struct Object3D {
    SDL_Texture *texture;
    f64 zrot;
    f64 xrot;
};

typedef struct {
    f64 x, y;
} vec2;
typedef struct {
    f64 x, y, z;
} vec3;