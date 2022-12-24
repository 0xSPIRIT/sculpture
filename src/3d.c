void object_rotate(struct Object3D *obj) {
    obj->zrot += 0.1f;
}

vec2* project(vec3 *input, int count) {
    vec2 *points = PushArray(gs->transient_memory, count, sizeof(vec2));
    
    for (int i = 0; i < count; i++) {
        if (input[i].z == 0) continue;
        points[i].x = input[i].x / input[i].z;
        points[i].x++; // Make it from 0 to 2
        points[i].x *= gs->window_width/2;  // Make it from 0 to W
        points[i].y = input[i].y / input[i].z;
        points[i].y++; // Make it from 0 to 2
        points[i].y *= gs->window_height/2; // Make it frmo 0 to H
    }
    
    return points;
}

void object_draw(struct Object3D *obj) {
    (void)obj;
    
    const int count = 4;
    
    vec3 *op = PushArray(gs->transient_memory, count, sizeof(vec3));
    
    op[0] = (vec3){-0.5, -0.5, 0};
    op[1] = (vec3){+0.5, -0.5, 0};
    op[3] = (vec3){+0.5, +0.5, 0};
    op[2] = (vec3){-0.5, +0.5, 0};
    
    vec3 *points = PushArray(gs->transient_memory, count, sizeof(vec3));
    
    // Rotation about the Y-axis
    
    for (int i = 0; i < count; i++) {
        f64 t = sin(SDL_GetTicks()/2000.0);
        
        points[i].x = cos(t) * op[i].x - sin(t) * op[i].z;
        points[i].y = op[i].y;
        points[i].z = sin(t) * op[i].x + cos(t) * op[i].z;
        
        points[i].z += 1; // Push it forward on the screen.
    }
    
    vec2 *projected = project(points, count);
    
    draw_image_skew(gs->surfaces.a, projected);
}