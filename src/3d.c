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
    
    bool draw_3d = false;
    if (!draw_3d) {
        return;
    }
    
    vec3 points[4];
    
    f64 z = 1.5+sin(SDL_GetTicks()/1000.0);
    
    points[0] = (vec3){+0.5, 0.5, z};
    points[1] = (vec3){-0.5, 0.5, z};
    points[2] = (vec3){+0.5, -0.5, z};
    points[3] = (vec3){-0.5, -0.5, z};
    
    vec2 *projected = project(points, 4);
    
    SDL_SetRenderDrawColor(gs->renderer, 255, 255, 255, 255);
    
    for (int i = 0; i < 4; i++) {
        fill_circle(gs->renderer, projected[i].x, projected[i].y, 5);
    }
}