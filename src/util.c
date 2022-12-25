bool is_angle_225(f64 deg_angle) {
    f64 f = fabs(deg_angle);
    if (f == 22.5 || f == 157.5 || f == 112.5 || f == 67.5) {
        return true;
    }
    return false;
}

bool is_angle_45(f64 deg_angle) {
    f64 f = fabs(deg_angle);
    if (f == 45 || f == 135 || f == 225 || f == 315) {
        return true;
    }
    return false;
}

bool is_in_bounds(int x, int y) {
    return x >= 0 && y >= 0 && x < gs->gw && y < gs->gh;
}

bool is_in_boundsf(f32 x, f32 y) {
    return is_in_bounds((int)x, (int)y);
}

void start_timer(void) {
    gs->global_start = clock();
}

void _end_timer(const char *func) {
    gs->global_end = clock();
    f64 cpu_time_used = ((f64) (gs->global_end-gs->global_start))/CLOCKS_PER_SEC;
    Log("(%s) Time: %f\n", func, cpu_time_used);
}

// Moves the mouse to the middle of the grid cell, not the top-left.
void move_mouse_to_grid_position(f32 x, f32 y) {
    SDL_WarpMouseInWindow(gs->window, (int)(x*gs->S + gs->S/2 - gs->view.x), GUI_H + (int)(y*gs->S + gs->S/2 - gs->view.y));
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

void get_name_from_type(int type, char *out) {
    switch (type) {
        case CELL_NONE:        strcpy(out, "nothing"); break;
        
        case CELL_DIRT:        strcpy(out, "Dirt"); break;
        case CELL_SAND:        strcpy(out, "Sand"); break;
        
        case CELL_WATER:       strcpy(out, "Water"); break;
        case CELL_ICE:         strcpy(out, "Ice"); break;
        case CELL_STEAM:       strcpy(out, "Steam"); break;
        
        case CELL_WOOD_LOG:    strcpy(out, "Wood Log"); break;
        case CELL_WOOD_PLANK:  strcpy(out, "Wood Plank"); break;
        
        case CELL_COBBLESTONE: strcpy(out, "Cobblestone"); break;
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

void get_name_from_tool(int type, char *out) {
    switch (type) {
        case TOOL_CHISEL_SMALL:  strcpy(out, "Small Chisel"); break;
        case TOOL_CHISEL_MEDIUM: strcpy(out, "Medium Chisel"); break;
        case TOOL_CHISEL_LARGE:  strcpy(out, "Large Chisel"); break;
        case TOOL_OVERLAY:       strcpy(out, "Overlay [o]"); break;
        case TOOL_BLOCKER:       strcpy(out, "Blocker"); break;
        case TOOL_DELETER:       strcpy(out, "Deleter"); break;
        case TOOL_PLACER:        strcpy(out, "Placer"); break;
        case TOOL_GRABBER:       strcpy(out, "Pointer"); break;
    }
}

void get_file_from_tool(int type, char *out) {
    switch (type) {
        case TOOL_CHISEL_SMALL:  strcpy(out, "chisel_small.png"); break;
        case TOOL_CHISEL_MEDIUM: strcpy(out, "chisel_medium.png"); break;
        case TOOL_CHISEL_LARGE:  strcpy(out, "chisel_large.png"); break;
        case TOOL_BLOCKER:       strcpy(out, "blocker.png"); break;
        case TOOL_OVERLAY:       strcpy(out, "overlay.png"); break;
        case TOOL_DELETER:       strcpy(out, "deleter.png"); break;
        case TOOL_PLACER:        strcpy(out, "placer.png"); break;
        case TOOL_GRABBER:       strcpy(out, "pointer.png"); break;
    }
}

// Stolen from stackoverflow somewhere.
void set_pixel(SDL_Surface *surface, int x, int y, Uint32 pixel) {
    Uint32 *const target_pixel = (Uint32 *) ((Uint8 *) surface->pixels
                                             + y * surface->pitch
                                             + x * surface->format->BytesPerPixel);
    *target_pixel = pixel;
}

SDL_Color get_pixel(SDL_Surface *surf, int x, int y) {
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

Uint32 get_pixel_int(SDL_Surface *surf, int x, int y) {
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

int my_rand(int seed) {
    return (_a * (seed * seed) + _c) % _m;
}

f32 my_rand_f32(int seed) {
    return (f32)my_rand(seed)/(f32)_m;
}

f32 randf(f32 size) {
    return size * ((f32)(rand()%RAND_MAX))/(f32)RAND_MAX;
}

int randR(int i) {
    Assert(i > 0);
    Assert(i < gs->gw* gs->gh);
    int num = gs->grid[i].rand;
    return my_rand(num*num);
}

int randG(int i) {
    int num = gs->grid[i].rand;
    return my_rand(my_rand(num*num));
}

int randB(int i) {
    int num = gs->grid[i].rand;
    return my_rand(my_rand(my_rand(num*num)));
}

int minimum(int a, int b) {
    if (a < b) return a;
    return b;
}

f32 lerp(f32 a, f32 b, f32 t) {
    return a + t*(b-a); // or a(1-t) + tb -- same thing.
}

int clamp(int a, int min, int max) {
    if (a < min) return min;
    if (a > max) return max;
    return a;
}

f32 clampf(f32 a, f32 min, f32 max) {
    if (a < min) return min;
    if (a > max) return max;
    return a;
}

f64 distance64(f64 ax, f64 ay, f64 bx, f64 by) {
    return sqrt((bx-ax)*(bx-ax) + (by-ay)*(by-ay));
}

f64 distance64_point(SDL_Point a, SDL_Point b) {
    return distance64((f64) a.x, (f64) a.y, (f64) b.x, (f64) b.y);
}

f32 distance(f32 ax, f32 ay, f32 bx, f32 by) {
    return sqrtf((bx-ax)*(bx-ax) + (by-ay)*(by-ay));
}

f32 distancei(int ax, int ay, int bx, int by) {
    return sqrtf((f32) ((bx-ax)*(bx-ax)) + (f32)((by-ay)*(by-ay)));
}

bool is_point_on_line(SDL_Point p, SDL_Point a, SDL_Point b) {
    return (distancei(a.x, a.y, p.x, p.y) + distancei(b.x, b.y, p.x, p.y)) == distancei(a.x, a.y, b.x, b.y);
}

// Taken from stackoverflow
SDL_Point closest_point_on_line(SDL_Point a, SDL_Point b, SDL_Point p) {
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

bool is_point_in_rect(SDL_Point p, SDL_Rect r) {
    return p.x >= r.x && p.x <= r.x+r.w && p.y >= r.y && p.y <= r.y+r.h;
}

int sign(int a) {
    return (a > 0) ? 1 : ((a == 0) ? 0 : -1);
}

// Stolen from https://stackoverflow.com/a/2049593
f32 sign_triangle(SDL_Point p1, SDL_Point p2, SDL_Point p3) {
    return (f32)(p1.x - p3.x) * (f32)(p2.y - p3.y) - (f32)(p2.x - p3.x) * (f32)(p1.y - p3.y);
}

bool is_point_in_triangle(SDL_Point pt, SDL_Point v1, SDL_Point v2, SDL_Point v3) {
    f32 d1, d2, d3;
    int has_neg, has_pos;
    
    d1 = sign_triangle(pt, v1, v2);
    d2 = sign_triangle(pt, v2, v3);
    d3 = sign_triangle(pt, v3, v1);
    
    has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
    has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);
    
    return !(has_neg && has_pos);
}

vec2 lerp_vec2(vec2 a, vec2 b, f64 t) {
    vec2 result;
    
    result.x = a.x + (b.x-a.x)*t;
    result.y = a.y + (b.y-a.y)*t;
    
    return result;
}

void fill_circle(SDL_Renderer *renderer, int x, int y, int size) {
    for (int yy = -size; yy <= size; yy++) {
        for (int xx = -size; xx <= size; xx++) {
            if (xx*xx + yy*yy > size*size) continue;
            
            SDL_RenderDrawPoint(renderer, x+xx, y+yy);
        }
    }
}

void draw_line_in_buffer(Uint32 *pixels, int w, int h, Uint32 color, int x1, int y1, int x2, int y2) {
#if 0
    (void)h;
    
    f64 dx=abs(x2-x1);
    f64 dy=abs(y2-y1);
    
    f64 step;
    
    if(dx>=dy)
        step=dx;
    else
        step=dy;
    
    dx=dx/step;
    dy=dy/step;
    
    f64 x=x1;
    f64 y=y1;
    
    int i=1;
    while(i<=step)
    {
        if (!pixels) {
            SDL_Color c;
            SDL_GetRGBA(color, gs->surfaces.bark_surface->format, &c.r, &c.g, &c.b, &c.a);
            SDL_SetRenderDrawColor(gs->renderer, c.r, c.g, c.b, c.a);
            SDL_RenderDrawPoint(gs->renderer, x, y);
        } else {
            pixels[(int)x+(int)y*w] = color;
        }
        x=x+dx;
        y=y+dy;
        i++;
    }
#endif
    
#if 1
    f64 dx = x2-x1;
    f64 dy = y2-y1;
    f64 len = sqrt(dx*dx+dy*dy);
    f64 ux = dx/len;
    f64 uy = dy/len;
    
    f64 xx = 0, yy = 0;
    f64 l = 0;
    
    while (l <= len) {
        int xxi = x1+xx;
        int yyi = y1+yy;
        
        if (xxi < 0 || yyi < 0 || xxi >= w || yyi >= h) {
            xx += ux;
            yy += uy;
            l = sqrt(xx*xx+yy*yy);
            continue;
        }
        
        pixels[xxi+yyi*w] = color;
        
        xx += ux;
        yy += uy;
        l = sqrt(xx*xx+yy*yy);
    }
#endif
}

void draw_text(TTF_Font *font,
               const char *str,
               SDL_Color col,
               SDL_Color bg_col,
               bool align_right,
               bool align_bottom,
               int x,
               int y,
               int *out_w,
               int *out_h) 
{
    SDL_Surface *surf = TTF_RenderText_LCD(font, str, col, bg_col);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(gs->renderer, surf);
    
    SDL_Rect dst = { x, y, surf->w, surf->h };
    
    if (align_right) dst.x -= surf->w;
    if (align_bottom) dst.y -= surf->h;
    
    if (out_w)
        *out_w = surf->w;
    if (out_h)
        *out_h = surf->h;
    
    SDL_SetRenderDrawColor(gs->renderer, 255, 255, 255, 255);
    SDL_RenderCopy(gs->renderer, texture, NULL, &dst);
    
    SDL_FreeSurface(surf);
    SDL_DestroyTexture(texture);
}

#if 0
void draw_surface(SDL_Surface *surf,
                  SDL_Rect o,
                  int x1, int y1,
                  int x2, int y2,
                  int x3, int y3,
                  int x4, int y4) 
{
    for (int y = 0; y < o.h; y++) {
        for (int x = 0; x < o.w; x++) {
            SDL_Color pixel = get_pixel(surf, x, y);
            
            
            
            SDL_SetRenderDrawColor(gs->renderer,
                                   pixel.r,
                                   pixel.g,
                                   pixel.b,
                                   pixel.a);
            //SDL_RenderDrawPoint(gs->renderer, xx, yy);
        }
    }
}
#endif

void fill_circle_in_buffer(Uint32 *buffer, Uint32 value, int x, int y, int w, int h, int size) {
    for (int yy = -size; yy <= size; yy++) {
        for (int xx = -size; xx <= size; xx++) {
            if (xx*xx + yy*yy > size*size) continue;
            if (x+xx < 0 || x+xx >= w) continue;
            if (y+yy < 0 || y+yy >= h) continue;
            
            buffer[x+xx+(y+yy)*w] = value;
        }
    }
}

void flood_fill_u32(Uint32 *buffer, Uint32 value, int w, int h, int x, int y, int *fill_stack) {
    int count = 1;
    fill_stack[0] = x+y*w;
    
    int c=0;
    
    while (count > 0) {
        int idx = fill_stack[--count];
        int dx = idx%w, dy = idx/w;
        
        if (dx < 0 || dy < 0 || dx >= w || dy >= h)
            continue;
        if (buffer[idx] == value)
            continue;
        
        buffer[idx] = value;
        c++;
        
        fill_stack[count++] = dx+1 + dy*w;
        fill_stack[count++] = dx-1 + dy*w;
        fill_stack[count++] = dx + (dy+1)*w;
        fill_stack[count++] = dx + (dy-1)*w;
    }
}

void fill_polygon_in_buffer(Uint32 *buffer, Uint32 value, int w, int h, vec2 *points, int n, int *fill_stack) {
    (void)fill_stack;
    
    vec2 prev = {-1, -1};
    vec2 avg = {0, 0};
    
    for (int i = 0; i < n; i++) {
        if (prev.x != -1)
            draw_line_in_buffer(buffer, w, h, value, prev.x, prev.y, points[i].x, points[i].y);
        
        avg.x += points[i].x;
        avg.y += points[i].y;
        
        prev = points[i];
    }
    draw_line_in_buffer(buffer, w, h, value, points[n-1].x, points[n-1].y, points[0].x, points[0].y);
    
    avg.x /= n;
    avg.y /= n;
    
    flood_fill_u32(buffer, value, w, h, avg.x, avg.y, fill_stack);
}

#define PROFILE 0

// Draw a surface given 4 points.
void draw_image_skew(SDL_Surface *surf, vec2 *p) {
#if PROFILE
    LARGE_INTEGER start;
    QueryPerformanceCounter(&start);
#endif
    
    int w = gs->window_width;
    int h = gs->window_height-GUI_H;
    
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
    
    int *fill_stack = PushArray(gs->transient_memory, w*h, sizeof(int)); // full of indices
    
    for (int y = 0; y < surf->h; y++) {
        vec2 prev = {-1, -1};
        for (int x = 0; x < surf->w; x++) {
            SDL_Color pixel = get_pixel(surf, x, y);
            Uint32 col = SDL_MapRGBA(surf->format, pixel.r, pixel.g, pixel.b, pixel.a);
            
            // t-values for x and y axes.
            f64 ty = (f64)y / (f64)surf->h;
            f64 tx = (f64)x / (f64)surf->w;
            
            vec2 point = lerp_vec2(lerp_vec2(p[0], p[2], ty),
                                   lerp_vec2(p[1], p[3], ty),
                                   tx);
            
            if (prev.x != -1 && y+1 < surf->h) {
                ty = (f64)(y+1) / (f64)surf->h;
                
                vec2 bottom_point = lerp_vec2(lerp_vec2(p[0], p[2], ty),
                                              lerp_vec2(p[1], p[3], ty),
                                              tx);
                
                vec2 pp[] = {
                    {prev.x, prev.y},
                    {point.x, point.y},
                    {bottom_point.x, bottom_point.y},
                    {bottom_point.x-(point.x-prev.x), bottom_point.y-(point.y-prev.y)},
                };
                
                fill_polygon_in_buffer(pixels, col, w, h, pp, 4, fill_stack);
            }
            
            prev = point;
        }
    }
    
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

void draw_line_225(f32 deg_angle, SDL_Point a, SDL_Point b, f32 size, bool infinite) {
    Assert(is_angle_225(deg_angle));
    
    int dir_x = sign(b.x - a.x);
    int dir_y = sign(b.y - a.y);
    
    if (dir_x == 0 && dir_y == 0) return;
    
    int aa = 0;
    int x = a.x;
    int y = a.y;
    
    f64 length = distance64((f64)a.x, (f64)b.y, (f64)x, (f64)y);
    f64 total_length = distance64_point(a, b);
    
    f64 prev_length = 0;
    
    Assert(is_in_boundsf(x, y));
    
    while (length < total_length || infinite) {
        bool break_out = false;
        
        aa = 0;
        while (aa < 2) {
            fill_circle(gs->renderer, x, y, size);
            
            f64 f = fabs(deg_angle);
            
            if (f == 22.5 || f == 157.5) {
                x += dir_x;
            } else {
                y += dir_y;
            }
            
            ++aa;
            length = distance64((f64)a.x, (f64)a.y, (f64)x, (f64)y);
            prev_length = length;
            
            if (!infinite && length > total_length) {
                break;
            }
        }
        
        if (break_out) break;
        
        f64 f = fabs(deg_angle);
        
        if (f == 22.5 || f == 157.5) {
            y += dir_y;
        } else {
            x += dir_x;
        }
        
        if (!is_in_boundsf(x, y)) {
            return;
        }
    }
}

void draw_line(SDL_Point a, SDL_Point b, f32 size, bool infinite) {
    f32 len = distancei(b.x, b.y, a.x, a.y);
    f32 dx = (f32) (b.x - a.x);
    f32 dy = (f32) (b.y - a.y);
    
    f32 ux, uy;
    
    ux = dx/len;
    uy = dy/len;
    
    SDL_FPoint c = {(f32)a.x, (f32)a.y};
    
    f32 curr_dist = 0;
    while (curr_dist < len || infinite) {
        fill_circle(gs->renderer, (int)c.x, (int)c.y, size);
        c.x += ux;
        c.y += uy;
        
        curr_dist = distance(c.x, c.y, (f32)a.x, (f32)a.y);
        
        if (infinite && !is_in_boundsf(c.x, c.y)) {
            return;
        }
    }
}
