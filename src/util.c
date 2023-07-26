static int sign(int a) {
    return (a > 0) ? 1 : ((a == 0) ? 0 : -1);
}

static f64 lerp64(f64 a, f64 b, f64 t) {
    f64 result = a + t*(b-a); // or a(1-t) + tb

    const f64 epsilon = 0.1;
    if (fabs(result-b) < epsilon) return b;

    return result;
}

static SDL_Color invert_color(SDL_Color c) {
    SDL_Color result;
    
    result.r = 255 - c.r;
    result.g = 255 - c.g;
    result.b = 255 - c.b;
    result.a = 255;
    
    return result;
}

static SDL_Color contrasting_color(SDL_Color c) {
    SDL_Color result;
    
    result.r = c.r > 127 ? 0 : 255;
    result.g = c.g > 127 ? 0 : 255;
    result.b = c.b > 127 ? 0 : 255;
    result.a = 255;
    
    return result;
}

// a = start, b = end, t = x-value (0 to 1)
static f64 interpolate64(f64 a, f64 b, f64 t) {
    if (fabs(b-a) < 50) {
        return a + 2 * sign(b-a);
    }
    return lerp64(a, b, t);
}

static f64 goto64(f64 a, f64 b, f64 t) {
    int x = sign(b-a);
    if (x > 0) {
        if (fabs(a+t*x) > fabs(b))
            return b;
    } else if (x < 0) {
        if (fabs(a+t*x) < fabs(b))
            return b;
    }
    return a + t*x;
}

static bool is_angle_225(f64 deg_angle) {
    f64 f = fabs(deg_angle);
    if (f == 22.5 || f == 157.5 || f == 112.5 || f == 67.5) {
        return true;
    }
    return false;
}

static bool ispunctuation(char c) {
    if (c == '.' || c == ',' || c == '-' || c == '?') return true;
    return false;
}

static vec2 vec2_mult(vec2 a, vec2 b) {
    return (vec2){a.x*b.x, a.y*b.y};
}

static vec2 vec2_scale(vec2 a, f32 scale) {
    return (vec2){a.x*scale, a.y*scale};
}

static vec2 vec2_add(vec2 a, vec2 b) {
    return (vec2){a.x+b.x, a.y+b.y};
}

static vec2 vec2_add3(const vec2 a, const vec2 b, const vec2 c) {
    return (vec2){a.x+b.x+c.x, a.y+b.y+c.y};
}

static vec3 vec3_add(vec3 a, vec3 b) {
    return (vec3){a.x+b.x, a.y+b.y, a.z+b.z};
}

static bool is_angle_45(f64 deg_angle) {
    f64 f = fabs(deg_angle);
    if (f == 45 || f == 135 || f == 225 || f == 315) {
        return true;
    }
    return false;
}

static bool is_in_bounds(int x, int y) {
    return x >= 0 && y >= 0 && x < gs->gw && y < gs->gh;
}

static bool is_in_boundsf(f32 x, f32 y) {
    return is_in_bounds((int)x, (int)y);
}

static bool is_in_view(int x, int y) {
    SDL_Rect view = {
        (int)(gs->render.view.x / gs->S),
        (int)(gs->render.view.y / gs->S),
        (int)(gs->render.view.w / gs->S),
        (int)(gs->render.view.h / gs->S),
    };
    
    // Hardcode
    if (gs->gw == 128) {
        view.x += 32;
    }
    
    bool result = (x >= view.x && y >= view.y && x <= view.x+view.w && y <= view.y+view.h);
    return result;
}

// Moves the mouse to the middle of the grid cell, not the top-left.
static void move_mouse_to_grid_position(f32 x, f32 y) {
    int new_x = (int)(x*gs->S + gs->S/2 - gs->render.view.x + (gs->real_width/2 - gs->game_width/2));
    int new_y = GUI_H + (int)(y*gs->S + gs->S/2 - gs->render.view.y + (gs->real_height/2 - gs->game_height/2));
    
    // Hardcode
    if (gs->gw == 128) new_x -= gs->game_width/2;
    
#if MOUSE_SIMULATED
    gs->input.real_mx = new_x;
    gs->input.real_my = new_y;
#else
    SDL_WarpMouseInWindow(gs->window, new_x, new_y);
#endif
}

void get_filename_from_type(int type, char *out) {
    switch (type) {
        case CELL_NONE:        strcpy(out, "nothing"); break;
        case CELL_DIRT:        strcpy(out, RES_DIR "items/dirt.png"); break;
        case CELL_SAND:        strcpy(out, RES_DIR "items/sand.png"); break;

        case CELL_WATER:       strcpy(out, RES_DIR "items/water.png"); break;
        case CELL_ICE:         strcpy(out, RES_DIR "items/ice.png"); break;
        case CELL_STEAM:       strcpy(out, RES_DIR "items/steam.png"); break;

        case CELL_WOOD_LOG:    strcpy(out, RES_DIR "items/wood_log.png"); break;
        case CELL_WOOD_PLANK:  strcpy(out, RES_DIR "items/wood_plank.png"); break;

        case CELL_STONE: strcpy(out, RES_DIR "items/cobblestone.png"); break;
        case CELL_MARBLE:      strcpy(out, RES_DIR "items/marble.png"); break;
        case CELL_SANDSTONE:   strcpy(out, RES_DIR "items/sandstone.png"); break;

        case CELL_CEMENT:      strcpy(out, RES_DIR "items/cement.png"); break;
        case CELL_CONCRETE:    strcpy(out, RES_DIR "items/concrete.png"); break;

        case CELL_QUARTZ:      strcpy(out, RES_DIR "items/quartz.png"); break;
        case CELL_GLASS:       strcpy(out, RES_DIR "items/glass.png"); break;

        case CELL_GRANITE:     strcpy(out, RES_DIR "items/granite.png"); break;
        case CELL_BASALT:      strcpy(out, RES_DIR "items/basalt.png"); break;
        case CELL_DIAMOND:     strcpy(out, RES_DIR "items/diamond.png"); break;

        case CELL_UNREFINED_COAL: strcpy(out, RES_DIR "items/unref_coal.png"); break;
        case CELL_REFINED_COAL:   strcpy(out, RES_DIR "items/ref_coal.png"); break;
        case CELL_LAVA:           strcpy(out, RES_DIR "items/lava.png"); break;

        case CELL_SMOKE:       strcpy(out, RES_DIR "items/smoke.png"); break;
        case CELL_DUST:        strcpy(out, RES_DIR "items/dust.png"); break;
    }
}

static void get_name_from_type(int type, char *out) {
    switch (type) {
        case CELL_NONE:        strcpy(out, "nothing"); break;

        case CELL_DIRT:        strcpy(out, "Dirt"); break;
        case CELL_SAND:        strcpy(out, "Sand"); break;

        case CELL_WATER:       strcpy(out, "Water"); break;
        case CELL_ICE:         strcpy(out, "Ice"); break;
        case CELL_STEAM:       strcpy(out, "Steam"); break;

        case CELL_WOOD_LOG:    strcpy(out, "Wood Log"); break;
        case CELL_WOOD_PLANK:  strcpy(out, "Wood Plank"); break;

        case CELL_STONE: strcpy(out, "Stone"); break;
        case CELL_MARBLE:      strcpy(out, "Marble"); break;
        case CELL_SANDSTONE:   strcpy(out, "Sandstone"); break;

        case CELL_CEMENT:      strcpy(out, "Cement"); break;
        case CELL_CONCRETE:    strcpy(out, "Concrete"); break;

        case CELL_QUARTZ:      strcpy(out, "Quartz"); break;
        case CELL_GLASS:       strcpy(out, "Glass"); break;

        case CELL_GRANITE:     strcpy(out, "Granite"); break;
        case CELL_BASALT:      strcpy(out, "Basalt"); break;
        case CELL_DIAMOND:     strcpy(out, "Diamond"); break;

        case CELL_UNREFINED_COAL: strcpy(out, "Unrefined Coal"); break;
        case CELL_REFINED_COAL: strcpy(out, "Refined Coal"); break;
        case CELL_LAVA:        strcpy(out, "Lava"); break;

        case CELL_SMOKE:       strcpy(out, "Smoke"); break;
        case CELL_DUST:        strcpy(out, "Dust"); break;
    }
}

static void get_name_from_tool(int type, char *out) {
    switch (type) {
        case TOOL_CHISEL_SMALL:  strcpy(out, "Small Chisel (1)"); break;
        case TOOL_CHISEL_MEDIUM: strcpy(out, "Medium Chisel (2)"); break;
        case TOOL_CHISEL_LARGE:  strcpy(out, "Large Chisel (3)"); break;
        case TOOL_OVERLAY:       strcpy(out, "Overlay (4)"); break;
        case TOOL_PLACER:        strcpy(out, "Placer (6)"); break;
        case TOOL_GRABBER:       strcpy(out, "Pointer (5)"); break;
        case TOOL_DESTROY:       strcpy(out, "Destroy Level"); break;
        case TOOL_FINISH_LEVEL:  strcpy(out, "Finish Level"); break;
    }
}

void get_file_from_tool(int type, char *out) {
    switch (type) {
        case TOOL_CHISEL_SMALL:  strcpy(out, "chisel_small.png"); break;
        case TOOL_CHISEL_MEDIUM: strcpy(out, "chisel_medium.png"); break;
        case TOOL_CHISEL_LARGE:  strcpy(out, "chisel_large.png"); break;
        //case TOOL_BLOCKER:       strcpy(out, "blocker.png"); break;
        case TOOL_OVERLAY:       strcpy(out, "overlay.png"); break;
        //case TOOL_DELETER:       strcpy(out, "deleter.png"); break;
        case TOOL_DESTROY:       strcpy(out, "destroy.png"); break;
        case TOOL_PLACER:        strcpy(out, "placer.png"); break;
        case TOOL_GRABBER:       strcpy(out, "pointer.png"); break;
        case TOOL_FINISH_LEVEL:  strcpy(out, "finish_level.png"); break;
    }
}

// Stolen from stackoverflow somewhere.
static void set_pixel(SDL_Surface *surface, int x, int y, Uint32 pixel) {
    Uint32 *target_pixel = (Uint32 *) ((Uint8 *) surface->pixels
                                       + y * surface->pitch
                                       + x * surface->format->BytesPerPixel);
    *target_pixel = pixel;
}

static SDL_Color get_pixel(SDL_Surface *surf, int x, int y) {
    if (x >= surf->w) x %= surf->w;
    if (y >= surf->h) y %= surf->h;
    int bpp = surf->format->BytesPerPixel;
    Assert(bpp == 4);

    Uint32 *pixels = (Uint32*)surf->pixels;
    SDL_Color color;

    Uint32 pixel = pixels[x+y*surf->w];

    SDL_GetRGBA(pixel, surf->format, &color.r, &color.g, &color.b, &color.a);

    return color;
}

static Uint32 get_pixel_int(SDL_Surface *surf, int x, int y) {
    if (x >= surf->w) x %= surf->w;
    if (y >= surf->h) y %= surf->h;
    int bpp = surf->format->BytesPerPixel;
    Assert(bpp == 4);
    Uint32 *pixels = (Uint32*)surf->pixels;
    return pixels[x+y*surf->w];
}


static int my_rand(int seed) {
    const int a = 1103515245;
    const int c = 12345;
    const int m = 2000000;
    return (a * (seed * seed) + c) % m;
}

static f32 my_rand_f32(int seed) {
    const int m = 2000000;
    return (f32)my_rand(seed)/(f32)m;
}

static f64 randf(f64 size) {
    return size * ((f64)(rand()%RAND_MAX))/(f64)RAND_MAX;
}

static int randR(int i) {
    Assert(i >= 0);
    Assert(i < gs->gw* gs->gh);
    int num = gs->grid[i].rand;
    return my_rand(num*num);
}

static int randG(int i) {
    int num = gs->grid[i].rand;
    return my_rand(my_rand(num*num));
}

static int randB(int i) {
    int num = gs->grid[i].rand;
    return my_rand(my_rand(my_rand(num*num)));
}

static f64 interpolate(f64 a, f64 b, f64 step) {
    if (a == b) return b;
    
    int sign = (b-a > 0) ? 1 : -1;
    
    if (sign > 0) {
        f64 result = a+step;
        if (result > b) return b;
        return result;
    } else {
        f64 result = a-step;
        if (result < b) return b;
        return result;
    }
}

static f64 clamp64(f64 x, f64 a, f64 b) {
    if (x < a) return a;
    if (x > b) return b;
    return x;
}

static f64 smoothstep(f64 x) {
    return x * x * (3.0 - 2.0 * x);
}

static f64 lerp(f64 a, f64 b, f64 t) {
    return a + t*(b-a); // or a(1-t) + tb -- same thing.
}

static f32 lerp_degrees(f32 start, f32 end, f32 amount) {
    f32 difference = abs(end - start);
    
    if (difference > 180) {
        // We need to add on to one of the values.
        if (end > start) {
            // We'll add it on to start...
            start += 360;
        } else {
            // Add it on to end.
            end += 360;
        }
    }
    
    // Interpolate it.
    f32 value = (start + ((end - start) * amount));
    
    // Wrap it..
    f32 rangeZero = 360;
    
    if (value >= 0 && value <= 360)
        return value;
    
    return fmod(value, rangeZero);
}

static f32 lerp_no_error(f32 a, f32 b, f32 t, f32 error) {
    f32 result = a + t*(b-a);
    if (abs(result - b) <= error)
        return b;
    return result;
}

static int clamp(int a, int min, int max) {
    if (a < min) return min;
    if (a > max) return max;
    return a;
}

static f64 clampf(f64 a, f64 min, f64 max) {
    if (a < min) return min;
    if (a > max) return max;
    return a;
}

static f64 distance64(f64 ax, f64 ay, f64 bx, f64 by) {
    return sqrt((bx-ax)*(bx-ax) + (by-ay)*(by-ay));
}

static f64 distance64_point(SDL_Point a, SDL_Point b) {
    return distance64((f64) a.x, (f64) a.y, (f64) b.x, (f64) b.y);
}

static f32 distance(f32 ax, f32 ay, f32 bx, f32 by) {
    return sqrtf((bx-ax)*(bx-ax) + (by-ay)*(by-ay));
}

static f32 distancei(int ax, int ay, int bx, int by) {
    return sqrtf((f32) ((bx-ax)*(bx-ax)) + (f32)((by-ay)*(by-ay)));
}

static bool is_point_on_line(SDL_Point p, SDL_Point a, SDL_Point b) {
    return (distancei(a.x, a.y, p.x, p.y) + distancei(b.x, b.y, p.x, p.y)) == distancei(a.x, a.y, b.x, b.y);
}

// Taken from stackoverflow
static SDL_Point closest_point_on_line(SDL_Point a, SDL_Point b, SDL_Point p) {
    SDL_Point AP = {p.x - a.x, p.y - a.y};
    SDL_Point AB = {b.x - a.x, b.y - a.y};

    f32 magnitudeAB = (f32) (AB.x*AB.x + AB.y*AB.y);
    f32 ABAPproduct = (f32) (AP.x*AB.x + AP.y*AB.y);
    f32 distance = ABAPproduct / magnitudeAB;

    if (distance < 0) {
        return a;
    } else if (distance > 1) {
        return b;
    } else {
        return (SDL_Point){(int) (a.x + AB.x * distance), (int) (a.y + AB.y * distance)};
    }
}

static bool is_point_in_rect(SDL_Point p, SDL_Rect r) {
    return p.x >= r.x && p.x <= r.x+r.w && p.y >= r.y && p.y <= r.y+r.h;
}

#if 0
static vec2 lerp_vec2(vec2 a, vec2 b, f64 t) {
    vec2 result;

    result.x = a.x + (b.x-a.x)*t;
    result.y = a.y + (b.y-a.y)*t;

    return result;
}
#endif

static void fill_circle_in_buffer(Uint32 *buffer, Uint32 value, int x, int y, int w, int h, int size) {
    for (int yy = -size; yy <= size; yy++) {
        for (int xx = -size; xx <= size; xx++) {
            if (xx*xx + yy*yy > size*size) continue;
            if (x+xx < 0 || x+xx >= w) continue;
            if (y+yy < 0 || y+yy >= h) continue;

            buffer[x+xx+(y+yy)*w] = value;
        }
    }
}
