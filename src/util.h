#ifndef UTIL_H_
#define UTIL_H_

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <stdbool.h>
#include <time.h>

#define end_timer() (_end_timer(__func__))

void start_timer();
void _end_timer(const char *func);

f32 randf(f32 size);
void move_mouse_to_grid_position(f32 x, f32 y);

void get_name_from_type(int type, char *out);
void get_name_from_tool(int type, char *out);
void get_file_from_tool(int type, char *out);
void get_filename_from_type(int type, char *out);

SDL_Color get_pixel(SDL_Surface *surf, int x, int y);
Uint32 get_pixel_int(SDL_Surface *surf, int x, int y);
void set_pixel(SDL_Surface *surface, int x, int y, Uint32 pixel);

int my_rand(int seed);
f32 my_rand_f32(int seed);

int minimum(int a, int b);

f32 lerp(f32 a, f32 b, f32 t);
int clamp(int a, int min, int max);
f32 clampf(f32 a, f32 min, f32 max);

f32 distance(f32 ax, f32 ay, f32 bx, f32 by);

bool is_point_on_line(SDL_Point p, SDL_Point a, SDL_Point b);
bool is_point_in_rect(SDL_Point p, SDL_Rect r);
bool is_point_in_triangle(SDL_Point pt, SDL_Point v1, SDL_Point v2, SDL_Point v3);

SDL_Point closest_point_on_line(SDL_Point a, SDL_Point b, SDL_Point p);
void draw_text(TTF_Font *font, const char *str, SDL_Color col, int align_right, int align_bottom, int x, int y, int *out_w, int *out_h);

#endif  /* UTIL_H_ */
