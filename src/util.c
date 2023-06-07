static int sign(int a) {
    return (a > 0) ? 1 : ((a == 0) ? 0 : -1);
}

static f64 lerp64(f64 a, f64 b, f64 t) {
    return a + t*(b-a); // or a(1-t) + tb -- same thing.
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

static void start_timer(void) {
    gs->global_start = clock();
}

static void _end_timer(const char *func) {
    gs->global_end = clock();
    f64 cpu_time_used = ((f64) (gs->global_end-gs->global_start))/CLOCKS_PER_SEC;
    Log("(%s) Time: %f\n", func, cpu_time_used);
}

// Moves the mouse to the middle of the grid cell, not the top-left.
static void move_mouse_to_grid_position(f32 x, f32 y) {
    SDL_WarpMouseInWindow(gs->window,
                          (int)(x*gs->S + gs->S/2 - gs->render.view.x + (gs->real_width/2 - gs->window_width/2)),
                          GUI_H + (int)(y*gs->S + gs->S/2 - gs->render.view.y + (gs->real_height/2 - gs->window_height/2)));
}

static void get_filename_from_type(int type, char *out) {
    switch (type) {
        case CELL_NONE:        strcpy(out, "nothing"); break;
        case CELL_DIRT:        strcpy(out, RES_DIR "items/dirt.png"); break;
        case CELL_SAND:        strcpy(out, RES_DIR "items/sand.png"); break;
        
        case CELL_WATER:       strcpy(out, RES_DIR "items/water.png"); break;
        case CELL_ICE:         strcpy(out, RES_DIR "items/ice.png"); break;
        case CELL_STEAM:       strcpy(out, RES_DIR "items/steam.png"); break;
        
        case CELL_WOOD_LOG:    strcpy(out, RES_DIR "items/wood_log.png"); break;
        case CELL_WOOD_PLANK:  strcpy(out, RES_DIR "items/wood_plank.png"); break;
        
        case CELL_COBBLESTONE: strcpy(out, RES_DIR "items/cobblestone.png"); break;
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
        
        case CELL_COBBLESTONE: strcpy(out, "Stone"); break;
        case CELL_MARBLE:      strcpy(out, "Marble"); break;
        case CELL_SANDSTONE:   strcpy(out, "Sandstone"); break;
        
        case CELL_CEMENT:      strcpy(out, "Cement"); break;
        case CELL_CONCRETE:    strcpy(out, "Concrete"); break;
        
        case CELL_QUARTZ:      strcpy(out, "Quartz"); break;
        case CELL_GLASS:       strcpy(out, "Glass"); break;
        
        case CELL_GRANITE:     strcpy(out, "Granite"); break;
        case CELL_BASALT:      strcpy(out, "Basalt"); break;
        case CELL_DIAMOND:     strcpy(out, "Diamond"); break;
        
        case CELL_UNREFINED_COAL: strcpy(out, "Unref. Coal"); break;
        case CELL_REFINED_COAL: strcpy(out, "Ref. Coal"); break;
        case CELL_LAVA:        strcpy(out, "Lava"); break;
        
        case CELL_SMOKE:       strcpy(out, "Smoke"); break;
        case CELL_DUST:        strcpy(out, "Dust"); break;
    }
}

static void get_name_from_tool(int type, char *out) {
    switch (type) {
        case TOOL_CHISEL_SMALL:  strcpy(out, "Small Chisel"); break;
        case TOOL_CHISEL_MEDIUM: strcpy(out, "Medium Chisel"); break;
        case TOOL_CHISEL_LARGE:  strcpy(out, "Large Chisel"); break;
        case TOOL_OVERLAY:       strcpy(out, "Overlay [O]"); break;
        //case TOOL_BLOCKER:       strcpy(out, "Blocker"); break;
        case TOOL_DELETER:       strcpy(out, "Deleter"); break;
        case TOOL_PLACER:        strcpy(out, "Placer"); break;
        case TOOL_GRABBER:       strcpy(out, "Pointer"); break;
        case TOOL_FINISH_LEVEL:  strcpy(out, "Finish Level"); break;
    }
}

static void get_file_from_tool(int type, char *out) {
    switch (type) {
        case TOOL_CHISEL_SMALL:  strcpy(out, "chisel_small.png"); break;
        case TOOL_CHISEL_MEDIUM: strcpy(out, "chisel_medium.png"); break;
        case TOOL_CHISEL_LARGE:  strcpy(out, "chisel_large.png"); break;
        //case TOOL_BLOCKER:       strcpy(out, "blocker.png"); break;
        case TOOL_OVERLAY:       strcpy(out, "overlay.png"); break;
        case TOOL_DELETER:       strcpy(out, "deleter.png"); break;
        case TOOL_PLACER:        strcpy(out, "placer.png"); break;
        case TOOL_GRABBER:       strcpy(out, "pointer.png"); break;
        case TOOL_FINISH_LEVEL:  strcpy(out, "finish_level.png"); break;
    }
}

// Stolen from stackoverflow somewhere.
static void set_pixel(SDL_Surface *surface, int x, int y, Uint32 pixel) {
    Uint32 *const target_pixel = (Uint32 *) ((Uint8 *) surface->pixels
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

const int _a = 1103515245;
const int _c = 12345;
const int _m = 2000000;

static int my_rand(int seed) {
    return (_a * (seed * seed) + _c) % _m;
}

static f32 my_rand_f32(int seed) {
    return (f32)my_rand(seed)/(f32)_m;
}

static f32 randf(f32 size) {
    return size * ((f32)(rand()%RAND_MAX))/(f32)RAND_MAX;
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

static f32 lerp(f32 a, f32 b, f32 t) {
    return a + t*(b-a); // or a(1-t) + tb -- same thing.
}

static int clamp(int a, int min, int max) {
    if (a < min) return min;
    if (a > max) return max;
    return a;
}

static f32 clampf(f32 a, f32 min, f32 max) {
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