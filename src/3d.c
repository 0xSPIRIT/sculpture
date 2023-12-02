// Vertices - array of 4 vertices that is set up for you. (out)
// Input - Array of 4 vectors in normalized (-1, 1) space.
static void project_square(SDL_Vertex *vertices, vec3 *input) {
    int count = 4;
    for (int i = 0; i < count; i++) {
        if (input[i].z == 0) continue;
        
        SDL_FPoint *pos = &vertices[i].position;
        
        pos->x = input[i].x / input[i].z;
        pos->x++; // Make it from 0 to 2.
        pos->x *= gs->game_width / 2.f; // 0 to gs->game_width
        
        pos->y = input[i].y / input[i].z;
        pos->y++; // Make it from 0 to 2.
        pos->y *= gs->game_height / 2.f; // 0 to gs->game_height
        
        if (i == 0 || i == 1) {
            pos->y += GUI_H;
        }
        
        vertices[i].color = WHITE;
    }
    
    vertices[0].tex_coord = (SDL_FPoint){0, 0};
    vertices[1].tex_coord = (SDL_FPoint){1, 0};
    vertices[2].tex_coord = (SDL_FPoint){0, 1};
    vertices[3].tex_coord = (SDL_FPoint){1, 1};
}

static void object_activate_new(Object3D *obj) {
    memset(obj, 0, sizeof(Object3D));
    gs->obj.z = 1;
    gs->obj.yrot = 0;
    gs->obj.active = true;

    SDL_Texture *prev = SDL_GetRenderTarget(gs->render.sdl);

    const int target = RENDER_TARGET_GRID;

    SDL_ShowCursor(SDL_DISABLE);

    RenderColor(0, 0, 0, 255);
    RenderClear(target);

    background_draw(target, &gs->background, -32, -32);
    grid_array_draw(target, 32, gs->grid, 255);
    
    SDL_Surface *surf = SDL_CreateRGBSurface(0, 64, 64, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
    SDL_RenderReadPixels(gs->renderer,
                         null,
                         ALASKA_PIXELFORMAT,
                         surf->pixels,
                         surf->pitch);
    
    SDL_SetRenderTarget(gs->renderer, prev);
    
    obj->texture = RenderCreateTextureFromSurface(surf);
    SDL_FreeSurface(surf);
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
    
    vec3 op[4];
    
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
            
        } break;
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
        } break;
    }
    
    vec3 points[4]; // Normalized in -1,1 coordinates
    
    for (int i = 0; i < 4; i++) {
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
    
    SDL_Vertex vertices[4];
    project_square(vertices, points);
    
    int indices[] = { 0, 1, 2, 1, 2, 3 };
    
    SDL_RenderGeometry(gs->renderer,
                       obj->texture.handle,
                       vertices,
                       4,
                       indices,
                       6);
}