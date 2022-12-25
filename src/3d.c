vec2* project(vec3 *input, int count) {
    vec2 *points = PushArray(gs->transient_memory, count, sizeof(vec2));
    
    for (int i = 0; i < count; i++) {
        if (input[i].z == 0) continue;
        points[i].x = input[i].x / input[i].z;
        points[i].x++; // Make it from 0 to 2
        points[i].x *= (gs->window_width)/2.0;  // Make it from 0 to W
        points[i].y = input[i].y / input[i].z;
        points[i].y++; // Make it from 0 to 2
        points[i].y *= (gs->window_height-GUI_H)/2.0; // Make it from 0 to H
    }
    
    return points;
}

void object_draw(struct Object3D *obj) {
    (void)obj;
    
    const int count = 4;
    
    vec3 *op = PushArray(gs->transient_memory, count, sizeof(vec3));
    
    op[0] = (vec3){-1, -1, obj->z};
    op[1] = (vec3){+1, -1, obj->z};
    op[2] = (vec3){-1, +1, obj->z};
    op[3] = (vec3){+1, +1, obj->z};
    
    f64 dy = 0.0002;
    
    switch (obj->state) {
        case OBJECT_ZOOM: {
            obj->y += dy;
                
            obj->z += 0.001;
            if (obj->z >= 2) {
                obj->state = OBJECT_ROTY;
            }
            break;
        }
        case OBJECT_ROTY: {
            obj->y += dy;
            
            for (int i = 0; i < count; i++)
                op[i].z = 0;
            
            obj->yrot -= 0.002;
            if (obj->yrot <= -0.6) {
                obj->state = OBJECT_FALL;
            }
            break;
        }
        case OBJECT_FALL: {
            for (int i = 0; i < count; i++)
                op[i].z = 0;
            
            if (obj->xrot > -1.5) {
                obj->jerk += 0.0000001;
                obj->acc += obj->jerk;
                obj->vel += obj->acc;
                obj->xrot -= obj->vel;
            }
            break;
        }
    }
    
    vec3 *points = PushArray(gs->transient_memory, count, sizeof(vec3));
    
    // Rotation about the Y-axis
    
    for (int i = 0; i < count; i++) {
        switch (obj->state) {
            case OBJECT_ZOOM: {
                points[i] = op[i];
                points[i].y += obj->y;
                break;
            }
            case OBJECT_ROTY: {
                f64 t = obj->yrot;
                points[i].x = cos(t) * op[i].x - sin(t) * op[i].z;
                points[i].y = op[i].y;
                points[i].z = sin(t) * op[i].x + cos(t) * op[i].z;
                
                points[i].z += obj->z; // Push it forward on the screen.
                points[i].y += obj->y;
                break;
            }
            case OBJECT_FALL: {
                f64 ty = -obj->yrot;
                f64 tx = obj->xrot;
                
                points[i].y -= 1;
                
                int x = op[i].x;
                int y = op[i].y;
                int z = op[i].z;
                
                points[i].x = z*cos(tx)*sin(ty) + y*sin(tx)*sin(ty) + x*cos(ty);
                points[i].y = y*cos(tx) - z*sin(tx);
                points[i].z = z*cos(tx)*cos(ty) - x*sin(ty) + y*sin(tx)*cos(ty);
                
                points[i].y += obj->y+1;
                points[i].z += obj->z; // Push it forward on the screen.
                break;
            }
        }
    }
    
    vec2 *projected = project(points, count);
    
#if 0
    for (int i = 0; i < count; i++) {
        projected[i].y += GUI_H;
    }
#endif
    
    draw_image_skew(gs->surfaces.a, projected);
    
    #if 0
    vec2 prev = {-1, -1};
    for (int i = 0; i < count; i++) {
        SDL_SetRenderDrawColor(gs->renderer, 255, 255, 255, 255);
        if (prev.x != -1)
            SDL_RenderDrawLine(gs->renderer, prev.x, prev.y, projected[i].x, projected[i].y);
        
        prev = projected[i];
    }
    SDL_RenderDrawLine(gs->renderer, projected[count-1].x, projected[count-1].y, projected[0].x, projected[0].y);
    #endif
}