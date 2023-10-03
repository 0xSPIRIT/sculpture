// Simple 3D Software Renderer

static inline void draw_triangle(int w, int h, SDL_Surface *surf, u32 *pixels, Vertex *p) {
    const vec2 t[3] = {p[0].p, p[1].p, p[2].p};
    const u32 *spixels = (u32*)surf->pixels;

    int sw = surf->w;
    int sh = surf->h;

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            f32 denominator = (t[1].y - t[2].y)*(t[0].x - t[2].x) + (t[2].x - t[1].x)*(t[0].y - t[2].y);

            f32 w0 = (t[1].y - t[2].y)*(x - t[2].x) + (t[2].x - t[1].x)*(y - t[2].y);
            if (w0*denominator < 0) continue;

            w0 /= denominator;

            f32 w1 = (t[2].y - t[0].y)*(x - t[2].x) + (t[0].x - t[2].x)*(y - t[2].y);
            if (w1*denominator < 0) continue; // If any weight < 0, the point is not in the triangle

            w1 /= denominator;

            f32 w2 = 1.00001f - w0 - w1; // Hack. Should be 1 , but weird glitches

            if (w2 < 0) continue; // If any weight < 0, the point is not in the triangle

            vec2 a = {p[0].tex.x*w0, p[0].tex.y*w0};
            vec2 b = {p[1].tex.x*w1, p[1].tex.y*w1};
            vec2 c = {p[2].tex.x*w2, p[2].tex.y*w2};

            vec2 tex_coord = (vec2){
                a.x+b.x+c.x,
                a.y+b.y+c.y
            };

            int xx = tex_coord.x * sw;
            int yy = tex_coord.y * sh;

            xx %= sw;
            yy %= sh;

            pixels[x+y*w] = spixels[xx+yy*surf->h];
        }
    }
}

// Draw a surface given 4 points.
static inline void draw_image_skew(int w, int h, SDL_Surface *surf, u32 *pixels, Vertex *p) {
    // We must use the vertex array as a set of
    // two triangles, in order to use classical
    // interpolation techniques for texture
    // coordinates and positions.

    draw_triangle(w, h, surf, pixels, p);
    draw_triangle(w, h, surf, pixels, p+1);
}

static void object_activate(Object3D *obj) {
    memset(obj, 0, sizeof(Object3D));
    gs->obj.z = 1;
    gs->obj.yrot = 0;
    gs->obj.active = true;

    SDL_Texture *prev = SDL_GetRenderTarget(gs->render.sdl);

    const int target = RENDER_TARGET_GRID;

    SDL_ShowCursor(SDL_DISABLE);

    RenderColor(0, 0, 0, 255);
    RenderClear(target);

    background_draw(target, &gs->background, 0, -32);
    grid_array_draw(target, gs->grid, 255);

    SDL_Rect rect = {
        32, 0,
        64, 64
    };

    void *pixels = PushArray(gs->transient_memory, gs->gh*gs->gh, 4); // sizeof(u32)
    SDL_RenderReadPixels(gs->renderer, &rect, ALASKA_PIXELFORMAT, pixels, gs->gh*4);

    gs->surfaces.a = SDL_CreateRGBSurfaceWithFormat(0,
                                                    gs->gh,
                                                    gs->gh,
                                                    32,
                                                    ALASKA_PIXELFORMAT);
    memcpy(gs->surfaces.a->pixels, pixels, gs->gh*gs->gh*4);

    SDL_SetRenderTarget(gs->renderer, prev);
}

static vec2* project(vec3 *input, int count) {
    vec2 *points = PushArray(gs->transient_memory, count, sizeof(vec2));

    for (int i = 0; i < count; i++) {
        if (input[i].z == 0) continue;

        points[i].x = input[i].x / input[i].z;
        points[i].x++; // Make it from 0 to 2
        points[i].x *= SCALE_3D * (gs->game_width)/2.f;  // Make it from 0 to W
        points[i].y = input[i].y / input[i].z;
        points[i].y++; // Make it from 0 to 2
        points[i].y *= SCALE_3D * (gs->game_height-GUI_H)/2.f; // Make it from 0 to H
    }

    return points;
}

static void object_draw(Object3D *obj) {
    if (!obj->active) return;

    if (obj->state == OBJECT_DONE) {
        if (obj->timer == -1) {
            narrator_tick();
            narrator_run(RENDER_TARGET_MASTER, BLACK);
            credits_run(RENDER_TARGET_MASTER);
        } else {
            obj->timer++;
        }

        if (obj->timer >= 6*60) { // 6*60
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

    switch (obj->state) {
        case OBJECT_ZOOM: {
            obj->t++;

            if (obj->hold == 0 && obj->z >= 2) {
                obj->hold = 20;
            } else if (obj->t > 120) {
                const f32 speed = 0.5;
                obj->z = 1 + smoothstep_2(speed * (obj->t-120) / 360.f);
                if (obj->z > 2) obj->z = 2;
                //obj->y += dy;
            }

            if (obj->hold) {
                obj->hold--;
                if (obj->hold == 0) {
                    obj->state = OBJECT_ROTY;
                    obj->z = 2;
                }
            }

            break;
        }
        case OBJECT_ROTY: case OBJECT_DONE: {
            if (obj->state != OBJECT_DONE) {
                //obj->y += dy;
                obj->t2 += 0.002f;
                obj->yrot = smoothstep_2(obj->t2);
                obj->yrot *= -M_PI/2;
            }

            if (obj->yrot <= -M_PI/2) {
                obj->state = OBJECT_DONE;
            }
            break;
        }
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
                f32 t = obj->yrot;

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
                f32 ty = -obj->yrot;
                f32 tx = obj->xrot;

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

    Vertex *final_points = PushArray(gs->transient_memory, 4, sizeof(Vertex));

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


    u32 *pixels;
    int pitch;
    if (RenderLockTexture(&RenderTarget(RENDER_TARGET_3D)->texture,
                          null,
                          (void**)&pixels,
                          &pitch) != 0) {
        Log("%s\n", SDL_GetError());
        Assert(0);
    }

    int w = SCALE_3D * gs->game_width;
    int h = SCALE_3D * (gs->game_height-GUI_H);

    memset(pixels, 0, pitch*h);

    draw_image_skew(w, h, gs->surfaces.a, pixels, final_points);

    RenderUnlockTexture(&RenderTarget(RENDER_TARGET_3D)->texture);

    SDL_Rect dst = {
        0, GUI_H,
        gs->game_width,
        gs->game_width
    };

    RenderMaybeSwitchToTarget(RENDER_TARGET_MASTER);
    SDL_RenderCopy(gs->render.sdl,
                   RenderTarget(RENDER_TARGET_3D)->texture.handle,
                   null,
                   &dst);
}
