#include "util.h"

#include <math.h>

#include "globals.h"

void get_name_from_type(int type, char *out) {
    switch (type) {
    case CELL_NONE:        strcpy(out, "nothing"); break;
    case CELL_MARBLE:      strcpy(out, "Marble"); break;
    case CELL_COBBLESTONE: strcpy(out, "Cobblestone"); break;
    case CELL_QUARTZ:      strcpy(out, "Quartz"); break;
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
    }
}

void get_name_from_tool(int type, char *out) {
    switch (type) {
    case TOOL_CHISEL_SMALL:  strcpy(out, "Small Chisel"); break;
    case TOOL_CHISEL_MEDIUM: strcpy(out, "Medium Chisel"); break;
    case TOOL_CHISEL_LARGE:  strcpy(out, "Large Chisel"); break;
    case TOOL_KNIFE:         strcpy(out, "Knife"); break;
    case TOOL_POINT_KNIFE:   strcpy(out, "Point Knife"); break;
    case TOOL_DRILL:         strcpy(out, "Drill"); break;
    case TOOL_PLACER:        strcpy(out, "Placer"); break;
    case TOOL_GRABBER:       strcpy(out, "Grabber"); break;
    }
}

void get_path_from_tool(int type, char *out) {
    switch (type) {
    case TOOL_CHISEL_SMALL:  strcpy(out, "../res/chisel_small.png"); break;
    case TOOL_CHISEL_MEDIUM: strcpy(out, "../res/chisel_medium.png"); break;
    case TOOL_CHISEL_LARGE:  strcpy(out, "../res/chisel_large.png"); break;
    case TOOL_KNIFE:         strcpy(out, "../res/knife.png"); break;
    case TOOL_POINT_KNIFE:   strcpy(out, "../res/point_knife.png"); break;
    case TOOL_DRILL:         strcpy(out, "../res/drill.png"); break;
    case TOOL_PLACER:        strcpy(out, "../res/placer.png"); break;
    case TOOL_GRABBER:       strcpy(out, "../res/pointer.png"); break;
    }
}

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
    Uint8 *pixels = surf->pixels;
    SDL_Color color;
    color.r = pixels[bpp * (x+y*surf->w) + 0];
    color.g = pixels[bpp * (x+y*surf->w) + 1];
    color.b = pixels[bpp * (x+y*surf->w) + 2];
    if (bpp == 4)
        color.a = pixels[bpp * (x+y*surf->w) + 3];
    return color;
}

static int a = 1103515245;
static int c = 12345;
static int m = 2000000;

int my_rand(int seed) {
    return (a * seed + c) % m;
}

int max(int a, int b) {
    if (a > b) return a;
    return b;
}

int min(int a, int b) {
    if (a < b) return a;
    return b;
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

float distance(SDL_Point a, SDL_Point b) {
    return sqrtf((b.x-a.x)*(b.x-a.x) + (b.y-a.y)*(b.y-a.y));
}

int is_point_on_line(SDL_Point p, SDL_Point a, SDL_Point b) {
    return (distance(a, p) + distance(b, p)) == distance(a, b);
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

int is_point_in_rect(SDL_Point p, SDL_Rect r) {
    if (p.x >= r.x && p.x <= r.x+r.w && p.y >= r.y && p.y <= r.y+r.h)
        return 1;
    return 0;
}

void draw_text(TTF_Font *font, const char *str, SDL_Color col, int align_left, int align_bottom, int x, int y, int *out_w, int *out_h) {
    SDL_Surface *surf = TTF_RenderText_Blended(font, str, col);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surf);
            
    SDL_Rect dst = { x, y, surf->w, surf->h };

    if (align_left) dst.x -= surf->w;
    if (align_bottom) dst.y -= surf->h;

    if (out_w)
        *out_w = surf->w;
    if (out_h)
        *out_h = surf->h;

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderCopy(renderer, texture, NULL, &dst);

    SDL_FreeSurface(surf);
    SDL_DestroyTexture(texture);
}
