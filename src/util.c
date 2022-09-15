#include "util.h"

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "globals.h"
#include "gui.h"
#include "grid.h"
#include "game.h"

void start_timer() {
    gs->global_start = clock();
}

void _end_timer(const char *func) {
    gs->global_end = clock();
    float cpu_time_used = ((double) (gs->global_end-gs->global_start))/CLOCKS_PER_SEC;
    SDL_Log("(%s) Time: %f\n", func, cpu_time_used); fflush(stdout);
}

// Moves the mouse to the middle of the grid cell, not the top-left.
void move_mouse_to_grid_position(float x, float y) {
    SDL_WarpMouseInWindow(gs->window, (int)(x*gs->S + gs->S/2), GUI_H + (int)(y*gs->S + gs->S/2));
}

void get_filename_from_type(int type, char *out) {
    switch (type) {
    case CELL_NONE:        strcpy(out, "nothing"); break;
    case CELL_MARBLE:      strcpy(out, "../res/items/marble.png"); break;
    case CELL_COBBLESTONE: strcpy(out, "../res/items/cobblestone.png"); break;
    case CELL_QUARTZ:      strcpy(out, "../res/items/quartz.png"); break;
    case CELL_GRANITE:     strcpy(out, "../res/items/quartz.png"); break;
    case CELL_BASALT:      strcpy(out, "../res/items/quartz.png"); break;
    case CELL_WOOD_LOG:    strcpy(out, "../res/items/wood_log.png"); break;
    case CELL_WOOD_PLANK:  strcpy(out, "../res/items/wood_plank.png"); break;
    case CELL_DIRT:        strcpy(out, "../res/items/dirt.png"); break;
    case CELL_SAND:        strcpy(out, "../res/items/sand.png"); break;
    case CELL_GLASS:       strcpy(out, "../res/items/glass.png"); break;
    case CELL_WATER:       strcpy(out, "../res/items/water.png"); break;
    case CELL_COAL:        strcpy(out, "../res/items/coal.png"); break;
    case CELL_STEAM:       strcpy(out, "../res/items/steam.png"); break;
    case CELL_DIAMOND:     strcpy(out, "../res/items/diamond.png"); break;
    case CELL_ICE:         strcpy(out, "../res/items/ice.png"); break;
    case CELL_LEAF:        strcpy(out, "../res/items/leaf.png"); break;
    case CELL_SMOKE:       strcpy(out, "../res/items/smoke.png"); break;
    case CELL_DUST:        strcpy(out, "../res/items/dust.png"); break;
    case CELL_LAVA:        strcpy(out, "../res/items/quartz.png"); break;
    }
}

void get_name_from_type(int type, char *out) {
    switch (type) {
    case CELL_NONE:        strcpy(out, "nothing"); break;
    case CELL_MARBLE:      strcpy(out, "Marble"); break;
    case CELL_COBBLESTONE: strcpy(out, "Cobblestone"); break;
    case CELL_QUARTZ:      strcpy(out, "Quartz"); break;
    case CELL_GRANITE:     strcpy(out, "Granite"); break;
    case CELL_BASALT:      strcpy(out, "Basalt"); break;
    case CELL_WOOD_LOG:    strcpy(out, "Wood Log"); break;
    case CELL_WOOD_PLANK:  strcpy(out, "Wood Plank"); break;
    case CELL_DIRT:        strcpy(out, "Dirt"); break;
    case CELL_SAND:        strcpy(out, "Sand"); break;
    case CELL_GLASS:       strcpy(out, "Glass"); break;
    case CELL_WATER:       strcpy(out, "Water"); break;
    case CELL_COAL:        strcpy(out, "Coal"); break;
    case CELL_STEAM:       strcpy(out, "Steam"); break;
    case CELL_DIAMOND:     strcpy(out, "Diamond"); break;
    case CELL_ICE:         strcpy(out, "Ice"); break;
    case CELL_LEAF:        strcpy(out, "Leaf"); break;
    case CELL_SMOKE:       strcpy(out, "Smoke"); break;
    case CELL_DUST:        strcpy(out, "Dust"); break;
    case CELL_LAVA:        strcpy(out, "Lava"); break;
    }
}

void get_name_from_tool(int type, char *out) {
    switch (type) {
    case TOOL_CHISEL_SMALL:  strcpy(out, "Small Chisel"); break;
    case TOOL_CHISEL_MEDIUM: strcpy(out, "Medium Chisel"); break;
    case TOOL_CHISEL_LARGE:  strcpy(out, "Large Chisel"); break;
    case TOOL_KNIFE:         strcpy(out, "Knife"); break;
    case TOOL_POINT_KNIFE:   strcpy(out, "Point Knife"); break;
    case TOOL_HAMMER:        strcpy(out, "Hammer"); break;
    case TOOL_PLACER:        strcpy(out, "Placer"); break;
    case TOOL_GRABBER:       strcpy(out, "Grabber"); break;
    }
}

void get_file_from_tool(int type, char *out) {
    switch (type) {
    case TOOL_CHISEL_SMALL:  strcpy(out, "chisel_small.png"); break;
    case TOOL_CHISEL_MEDIUM: strcpy(out, "chisel_medium.png"); break;
    case TOOL_CHISEL_LARGE:  strcpy(out, "chisel_large.png"); break;
    case TOOL_KNIFE:         strcpy(out, "knife.png"); break;
    case TOOL_POINT_KNIFE:   strcpy(out, "point_knife.png"); break;
    case TOOL_HAMMER:        strcpy(out, "hammer.png"); break;
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
    Assert(gs->window, bpp == 4);

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
    Assert(gs->window, bpp == 4);
    Uint32 *pixels = (Uint32*)surf->pixels;
    return pixels[x+y*surf->w];
}

internal int a = 1103515245;
internal int c = 12345;
internal int m = 2000000;

int my_rand(int seed) {
    return (a * seed + c) % m;
}

float my_rand_float(int seed) {
    return (float)my_rand(seed)/(float)m;
}

float randf(float size) {
    return size * ((float)(rand()%RAND_MAX))/(float)RAND_MAX;
}

int minimum(int a, int b) {
    if (a < b) return a;
    return b;
}

float lerp(float a, float b, float t) {
    return a + t*(b-a); // or a(1-t) + tb -- same thing.
}

int clamp(int a, int min, int max) {
    if (a < min) return min;
    if (a > max) return max;
    return a;
}

float clampf(float a, float min, float max) {
    if (a < min) return min;
    if (a > max) return max;
    return a;
}

float distance(float ax, float ay, float bx, float by) {
    return sqrtf((bx-ax)*(bx-ax) + (by-ay)*(by-ay));
}

bool is_point_on_line(SDL_Point p, SDL_Point a, SDL_Point b) {
    return (distance(a.x, a.y, p.x, p.y) + distance(b.x, b.y, p.x, p.y)) == distance(a.x, a.y, b.x, b.y);
}

// Taken from stackoverflow
SDL_Point closest_point_on_line(SDL_Point a, SDL_Point b, SDL_Point p) {
    SDL_Point AP = {p.x - a.x, p.y - a.y};
    SDL_Point AB = {b.x - a.x, b.y - a.y};

    float magnitudeAB = AB.x*AB.x + AB.y*AB.y;
    float ABAPproduct = AP.x*AB.x + AP.y*AB.y;
    float distance = ABAPproduct / magnitudeAB;

    if (distance < 0) {
        return a;
    } else if (distance > 1) {
        return b;
    } else {
        return (SDL_Point){a.x + AB.x * distance, a.y + AB.y * distance};
    }
}

bool is_point_in_rect(SDL_Point p, SDL_Rect r) {
    return p.x >= r.x && p.x <= r.x+r.w && p.y >= r.y && p.y <= r.y+r.h;
}

// Stolen from https://stackoverflow.com/a/2049593
internal float sign(SDL_Point p1, SDL_Point p2, SDL_Point p3) {
    return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
}

bool is_point_in_triangle(SDL_Point pt, SDL_Point v1, SDL_Point v2, SDL_Point v3) {
    float d1, d2, d3;
    int has_neg, has_pos;

    d1 = sign(pt, v1, v2);
    d2 = sign(pt, v2, v3);
    d3 = sign(pt, v3, v1);

    has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
    has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

    return !(has_neg && has_pos);
}

void draw_text(TTF_Font *font, const char *str, SDL_Color col, int align_left, int align_bottom, int x, int y, int *out_w, int *out_h) {
    SDL_Surface *surf = TTF_RenderText_Blended(font, str, col);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(gs->renderer, surf);
            
    SDL_Rect dst = { x, y, surf->w, surf->h };

    if (align_left) dst.x -= surf->w;
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
