//
// Simple software renderer.
//

struct TriangleDrawData {
    int start_y, end_y;
    Uint32 *pixels;
    SDL_Surface *surf;
    int w;
    Vertex points[3];
};


// 3 vertices required
inline void draw_triangle_row(Uint32 *pixels, SDL_Surface *surf, int w, int y, Vertex *p) {
    const vec2 t[3] = {p[0].p, p[1].p, p[2].p};
    
    for (int x = 0; x < w; x++) {
        f32 denominator = (t[1].y - t[2].y)*(t[0].x - t[2].x) + (t[2].x - t[1].x)*(t[0].y - t[2].y);
        
        f32 w0 = (t[1].y - t[2].y)*(x - t[2].x) + (t[2].x - t[1].x)*(y - t[2].y);
        w0 /= denominator;
        
        if (w0 < 0) continue; // If any weight < 0, the point is not in the triangle
        
        f32 w1 = (t[2].y - t[0].y)*(x - t[2].x) + (t[0].x - t[2].x)*(y - t[2].y);
        w1 /= denominator;
        
        if (w1 < 0) continue; // If any weight < 0, the point is not in the triangle
        
        f32 w2 = 1 - w0 - w1;
        
        if (w2 < 0) continue; // If any weight < 0, the point is not in the triangle
        
#if 0
        vec3 color;
        color.x = w0 * p[0].col.x + w1 * p[1].col.x + w2 * p[2].col.x;
        color.y = w0 * p[0].col.y + w1 * p[1].col.y + w2 * p[2].col.y;
        color.z = w0 * p[0].col.z + w1 * p[1].col.z + w2 * p[2].col.z;
        
        Uint32 pixel = SDL_MapRGB(format, color.x, color.y, color.z);
#endif
        
        //vec2 tex_coord = vec2_add3(vec2_scale(p[0].tex, w0),
        //                           vec2_scale(p[1].tex, w1),
        //                           vec2_scale(p[2].tex, w2));
        
        f32 tex_coord_x = p[0].tex.x * w0 + p[1].tex.x * w1 + p[2].tex.x * w2;
        f32 tex_coord_y = p[0].tex.y * w0 + p[1].tex.y * w1 + p[2].tex.y * w2;
        
        int xx = tex_coord_x * surf->w;
        int yy = tex_coord_y * surf->h;
        
        if (yy >= surf->h) yy = surf->h-1;
        if (xx >= surf->w) xx = surf->w-1;
        
        pixels[x+y*w] = ((Uint32*)(surf->pixels))[xx+yy*surf->w];
    }
}

inline void draw_triangle(void *ptr) {
    struct TriangleDrawData *data = (struct TriangleDrawData*)ptr;
    
    for (int y = data->start_y; y < data->end_y; y++) {
        draw_triangle_row(data->pixels, data->surf, data->w, y, data->points);
    }
}

#define PROFILE 0

// Draw a surface given 4 points.
void draw_image_skew(int w, int h, SDL_Surface *surf, Vertex *p) {
#if PROFILE
    LARGE_INTEGER start;
    QueryPerformanceCounter(&start);
#endif
    
    Uint32 *pixels;
    int pitch;
    if (SDL_LockTexture(RenderTarget(RENDER_TARGET_3D),
                        NULL,
                        &pixels,
                        &pitch) != 0) {
        Log("%s\n", SDL_GetError());
        Assert(0);
    }
    
    ZeroMemory(pixels, pitch*h);
    
    // We must use the vertex array as a set of
    // two triangles, in order to use classical
    // interpolation techniques for texture
    // coordinates and positions.
    
    struct TriangleDrawData data = {
        0, h,
        pixels,
        surf,
        w,
        {p[0], p[1], p[2]}
    };
    draw_triangle(&data);
    
    data = (struct TriangleDrawData){
        0, h,
        pixels,
        surf,
        w,
        {p[1], p[2], p[3]}
    };
    draw_triangle(&data);
    
    SDL_UnlockTexture(RenderTarget(RENDER_TARGET_3D));
    
#if PROFILE
    LARGE_INTEGER end;
    QueryPerformanceCounter(&end);
    
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    
    f64 d = (end.QuadPart - start.QuadPart) / (f64) frequency.QuadPart;
    
    Log("Function: %f ms\n", d*1000);
#endif
}

void object_init(struct Object3D *obj) {
    memset(obj, 0, sizeof(struct Object3D));
    gs->obj.z = 1;
    gs->obj.active = true;
    gs->obj.yrot = -0.002f;
    
    Uint32 *pixels = PushArray(gs->persistent_memory, gs->gw*gs->gh, sizeof(Uint32));
    
    int pitch = 4*gs->gw;
    SDL_Texture *prev = SDL_GetRenderTarget(gs->renderer);
    SDL_SetRenderTarget(gs->renderer, RenderTarget(RENDER_TARGET_GLOBAL));
    SDL_RenderReadPixels(gs->renderer, NULL, ALASKA_PIXELFORMAT, pixels, pitch);
    SDL_SetRenderTarget(gs->renderer, prev);
    
    obj->surf = SDL_CreateRGBSurfaceWithFormat(0,
                                               gs->gw,
                                               gs->gh,
                                               32,
                                               ALASKA_PIXELFORMAT);
    Assert(obj->surf);
    memcpy(obj->surf->pixels, pixels, sizeof(Uint32)*gs->gw*gs->gh);
}

vec2* project(vec3 *input, int count) {
    vec2 *points = PushArray(gs->transient_memory, count, sizeof(vec2));
    
    for (int i = 0; i < count; i++) {
        if (input[i].z == 0) continue;
        points[i].x = input[i].x / input[i].z;
        points[i].x++; // Make it from 0 to 2
        points[i].x *= SCALE_3D * (gs->window_width)/2.0;  // Make it from 0 to W
        points[i].y = input[i].y / input[i].z;
        points[i].y++; // Make it from 0 to 2
        points[i].y *= SCALE_3D * (gs->window_height-GUI_H)/2.0; // Make it from 0 to H
    }
    
    return points;
}

void object_draw(void *ptr) {
    struct Object3D *obj = (struct Object3D*)ptr;
    const int count = 4;
    
    vec3 op[4] = {0};
    
    op[0] = (vec3){-1, -1, obj->z};
    op[1] = (vec3){+1, -1, obj->z};
    op[2] = (vec3){-1, +1, obj->z};
    op[3] = (vec3){+1, +1, obj->z};
    
    f64 dy = 0.0002;
    
    switch (obj->state) {
        case OBJECT_ZOOM: {
            obj->y += dy;
            
            obj->z += 0.01f;
            if (obj->z >= 2) {
                obj->state = OBJECT_ROTY;
            }
            break;
        }
        case OBJECT_ROTY: case OBJECT_DONE: {
            obj->y += dy;
            
            for (int i = 0; i < count; i++)
                op[i].z = 0;
            
            const f32 speed = -0.003f;
            
            obj->yrot += speed;
            break;
        }
        case OBJECT_FALL: {
            for (int i = 0; i < count; i++)
                op[i].z = 0;
            
            if (obj->xrot > -1.5) {
                obj->jerk += 0.0000001f;
                obj->acc += obj->jerk;
                obj->vel += obj->acc;
                obj->xrot -= obj->vel;
            }
            break;
        }
    }
    
    vec3 points[4] = {0};
    
    // Rotation about the Y-axis
    
    for (int i = 0; i < count; i++) {
        switch (obj->state) {
            case OBJECT_ZOOM: {
                points[i] = op[i];
                points[i].y += obj->y;
                break;
            }
            case OBJECT_ROTY: case OBJECT_DONE: {
                f64 t = obj->yrot;
                
                points[i].x = cos(t) * op[i].x - sin(t) * op[i].z;
                points[i].y = op[i].y;
                points[i].z = sin(t) * op[i].x + cos(t) * op[i].z;
                
                points[i].y += obj->y;
                points[i].z += obj->z; // Push it forward on the screen.
                break;
            }
            case OBJECT_FALL: {
                f64 ty = -obj->yrot;
                f64 tx = obj->xrot;
                
                int x = op[i].x;
                int y = op[i].y;
                int z = op[i].z;
                
                points[i].x = z*cos(tx)*sin(ty) + y*sin(tx)*sin(ty) + x*cos(ty);
                points[i].y = y*cos(tx) - z*sin(tx);
                points[i].z = z*cos(tx)*cos(ty) - x*sin(ty) + y*sin(tx)*cos(ty);
                
                points[i].y += obj->y;
                points[i].z += obj->z; // Push it forward on the screen.
                break;
            }
        }
    }
    
    
    for (int i = 0; i < 4; i++) {
        Assert(points[i].z >= 1);
    }
    
    vec2 *projected = project(points, count);
    
#if 0
    for (int i = 0; i < count; i++) {
        projected[i].y += GUI_H;
    }
#endif
    
    Vertex final_points[4] = {0};
    
    for (int i = 0; i < count; i++) {
        final_points[i].p.x = projected[i].x;
        final_points[i].p.y = projected[i].y;
    }
    
    final_points[0].tex.x = 0;
    final_points[0].tex.y = 0;
    
    final_points[1].tex.x = 1;
    final_points[1].tex.y = 0;
    
    final_points[2].tex.x = 0;
    final_points[2].tex.y = 1;
    
    final_points[3].tex.x = 1;
    final_points[3].tex.y = 1;
    
    
    int w = SCALE_3D * gs->window_width;
    int h = SCALE_3D * (gs->window_height-GUI_H);
    
    Assert(w==h);
    
    draw_image_skew(w, h, gs->surfaces.grass_surface, final_points);
}

void snow_init(struct Snow3D *snow) {
    for (int i = 0; i < SNOW_3D_PARTICLE_COUNT; i++) {
        snow->p[i] = (vec3){
            my_rand_f32(my_rand(my_rand(i))),
            my_rand_f32(my_rand(i)),
            1.25+my_rand_f32(my_rand(my_rand(my_rand(i))))
        };
        snow->v[i] = (vec3){
            0.005f,
            0.005f,
            0.00f
        };
    }
}

void sinow_draw(struct Snow3D *snow) {
    struct Object3D *obj = &gs->obj;
    
    for (int i = 0; i < SNOW_3D_PARTICLE_COUNT; i++) {
        snow->p[i] = vec3_add(snow->p[i], snow->v[i]);
        
        vec3 p = snow->p[i];
        
        f64 t = obj->yrot;
        
        // Z-values from 1.25 to 2.25.
        // We want to rotate about the origin,
        // so we subtract 1.75 from the z-values.
        // Then, the z-values would be -0.5 to +0.5.
        
        p.z -= 1.75;
        
        p.x = cos(t) * p.x - sin(t) * p.z;
        p.y = p.y;
        p.z = sin(t) * p.x + cos(t) * p.z;
        
        // Then, we re-add the 1.75 to get it back to where it was before.
        p.z += 1.75;
        
        vec2 projected = *project(&p, 1);
        
        if (projected.x >= gs->window_width) {
            snow->p[i].x = -snow->p[i].z;
        }
        if (projected.y >= gs->window_height) {
            snow->p[i].y = -snow->p[i].z;
        }
        
        const int size = 3;
        
        SDL_SetRenderDrawColor(gs->renderer, 255, 255, 255, 255);
        f32 z = p.z;
        z--;
        z *= size;
        z = size-z;
        fill_circle(gs->renderer, projected.x, projected.y, (int)z);
    }
}
