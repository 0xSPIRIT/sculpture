#ifndef RENDER_H_
#define RENDER_H_ 

struct Ray {
    float x, y, z;
    float vx, vy, vz; // Direction as unit vector
    float light_strength; // Decreases as number of bounces increase.
};

enum Light_Type {
    LIGHT_SUN,
    LIGHT_POINT
};
 
struct Light {
    int type;
    struct Ray *rays; // Dynamically allocated array.
    int ray_count, ray_capacity;
};

#endif // RENDER_H_
