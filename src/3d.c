//
// Simple 3D Software Renderer
//

struct TriangleDrawData {
    int start_y, end_y;
    Uint32 *pixels;
    SDL_Surface *surf;
    int w;
    Vertex points[3];
};


// 3 vertices required
void draw_triangle_row(Uint32 *pixels, SDL_Surface *surf, int w, int y, Vertex *p) {
    const vec2 t[3] = {p[0].p, p[1].p, p[2].p};
    const SDL_PixelFormat *format = surf->format;
    
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
        
        vec2 tex_coord = vec2_add3(vec2_scale(p[0].tex, w0),
                                   vec2_scale(p[1].tex, w1),
                                   vec2_scale(p[2].tex, w2));
        
        SDL_Color c = get_pixel(surf, tex_coord.x * surf->w, tex_coord.y * surf->h);
        Uint32 pixel = SDL_MapRGB(format, c.r, c.g, c.b);
        
        pixels[x+y*w] = pixel;
    }
}

void draw_triangle(void *ptr) {
    struct TriangleDrawData *data = (struct TriangleDrawData*)ptr;
    
    for (int y = data->start_y; y < data->end_y; y++) {
        draw_triangle_row(data->pixels, data->surf, data->w, y, data->points);
    }
}

#define PROFILE 0

// Draw a surface given 4 points.
void draw_image_skew(int w, int h, SDL_Surface *surf, Uint32 *pixels, Vertex *p) {
#if PROFILE
    LARGE_INTEGER start;
    QueryPerformanceCounter(&start);
#endif
    
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
    
#if PROFILE
    LARGE_INTEGER end;
    QueryPerformanceCounter(&end);
    
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    
    f64 d = (end.QuadPart - start.QuadPart) / (f64) frequency.QuadPart;
    
    Log("Function: %f ms\n", d*1000);
#endif
}

void object_activate(struct Object3D *obj) {
    memset(obj, 0, sizeof(struct Object3D));
    gs->obj.z = 1;
    gs->obj.yrot = -0.002;
    gs->obj.active = true;
    
    SDL_Texture *prev = SDL_GetRenderTarget(gs->renderer);
    
#if 0
    // When taking the pixels directly from the screen, and
    // memcpying it into the surface's pixel buffer.
    SDL_SetRenderTarget(gs->renderer, RenderTarget(RENDER_TARGET_GLOBAL));
#else
    // Drawing the grid's pixels to a texture, then
    // memcpying that into the surface's pixel buffer.
    
    SDL_SetRenderTarget(gs->renderer, RenderTarget(RENDER_TARGET_GRID));
    SDL_SetRenderDrawColor(gs->renderer, 0, 0, 0, 255);
    SDL_RenderClear(gs->renderer);
    
    grid_array_draw(gs->grid, 255);
#endif
    
    void *pixels = PushArray(gs->transient_memory, gs->gw*gs->gh, 4); // sizeof(Uint32)
    SDL_RenderReadPixels(gs->renderer, NULL, ALASKA_PIXELFORMAT, pixels, gs->gw*4);
    
    gs->surfaces.a = SDL_CreateRGBSurfaceWithFormat(0,
                                                    gs->gw,
                                                    gs->gh,
                                                    32,
                                                    ALASKA_PIXELFORMAT);
    memcpy(gs->surfaces.a->pixels, pixels, gs->gw*gs->gh*4);
    
    SDL_SetRenderTarget(gs->renderer, prev);
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

void object_draw(struct Object3D *obj) {
    if (!obj->active) return;
    if (obj->state == OBJECT_DONE) {
        if (obj->timer == -1) {
            narrator_tick();
            narrator_run(BLACK);
            credits_run();
        } else {
            obj->timer++;
        }
        
        if (obj->timer >= 6*60) {
            narrator_init(11);
            obj->timer = -1;
        }
        return;
    }
    
    const int count = 4;
    
    vec3 *op = PushArray(gs->transient_memory, count, sizeof(vec3));
    
    op[0] = (vec3){-1, -1, obj->z};
    op[1] = (vec3){+1, -1, obj->z};
    op[2] = (vec3){-1, +1, obj->z};
    op[3] = (vec3){+1, +1, obj->z};
    
    f64 dy = 0.0002;
    
#if 0
    if (gs->input.keys[SDL_SCANCODE_SPACE]) {
        obj->state = OBJECT_DONE;
    }
#endif
    
    const f64 speed = 1.5f;
    
    switch (obj->state) {
        case OBJECT_ZOOM: {
            obj->y += dy;
            
            obj->z += speed * 0.001;
            if (obj->z >= 2) {
                obj->state = OBJECT_ROTY;
                obj->z = 2;
            }
            break;
        }
        case OBJECT_ROTY: case OBJECT_DONE: {
            if (obj->state != OBJECT_DONE) {
                obj->y += dy;
                obj->yrot -= speed * 0.002;
            }
            
            if (obj->yrot <= -M_PI/2) {
                obj->state = OBJECT_DONE;
            }
            break;
        }
#if 0
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
#endif
    }
    
    vec3 *points = PushArray(gs->transient_memory, count, sizeof(vec3));
    
    for (int i = 0; i < count; i++) {
        switch (obj->state) {
            case OBJECT_ZOOM: {
                points[i] = op[i];
                points[i].y += obj->y;
                break;
            }
            case OBJECT_ROTY: case OBJECT_DONE: {
                f64 t = obj->yrot;
                
                // We comment out the Z part because we're rotating
                // about the point 0,0,
                
                points[i].x = cos(t) * op[i].x;// - sin(t) * op[i].z;
                points[i].y = op[i].y;
                points[i].z = sin(t) * op[i].x;// + cos(t) * op[i].z;
                
                // Then we're adding back the Z depth by
                points[i].y += obj->y;
                points[i].z += obj->z; // pushing it forward on the screen.
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
    
    vec2 *projected = project(points, count);
    
#if 0
    for (int i = 0; i < count; i++) {
        projected[i].y += GUI_H;
    }
#endif
    
    Vertex *final_points = PushArray(gs->transient_memory, 4, sizeof(Vertex));
    
    for (int i = 0; i < count; i++) {
        final_points[i].p.x = projected[i].x;
        final_points[i].p.y = projected[i].y;
    }
    
#if 0
    final_points[0].col.x = 255;
    final_points[0].col.y = 0;
    final_points[0].col.z = 0;
    
    final_points[1].col.x = 0;
    final_points[1].col.y = 255;
    final_points[1].col.z = 0;
    
    final_points[2].col.x = 0;
    final_points[2].col.y = 0;
    final_points[2].col.z = 255;
#endif

    final_points[0].tex.x = 0;
    final_points[0].tex.y = 0;
    
    final_points[1].tex.x = 1;
    final_points[1].tex.y = 0;
    
    final_points[2].tex.x = 0;
    final_points[2].tex.y = 1;
    
    final_points[3].tex.x = 1;
    final_points[3].tex.y = 1;
    
    
    Uint32 *pixels;
    int pitch;
    if (SDL_LockTexture(RenderTarget(RENDER_TARGET_3D),
                        NULL,
                        &pixels,
                        &pitch) != 0) {
        Log("%s\n", SDL_GetError());
        Assert(0);
    }
    
    int w = SCALE_3D * gs->window_width;
    int h = SCALE_3D * (gs->window_height-GUI_H);
    
#ifdef __EMSCRIPTEN__
    memset(pixels, 0, pitch*h);
#else
    ZeroMemory(pixels, pitch*h);
#endif
    
    draw_image_skew(w, h, gs->surfaces.a, pixels, final_points);
    draw_image_skew(w, h, gs->surfaces.a, pixels, final_points+1);
    
    SDL_UnlockTexture(RenderTarget(RENDER_TARGET_3D));
    
    const SDL_Rect dst = {
        0, GUI_H,
        gs->window_width, gs->window_height-GUI_H
    };
    SDL_RenderCopy(gs->renderer, RenderTarget(RENDER_TARGET_3D), NULL, &dst);
}