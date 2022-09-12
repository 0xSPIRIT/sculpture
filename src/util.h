#ifndef UTIL_H_
#define UTIL_H_

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <stdbool.h>
#include <time.h>

#define end_timer() (_end_timer(__func__))

void start_timer();
void _end_timer(const char *func);

float randf(float size);
void move_mouse_to_grid_position(float x, float y);

void get_name_from_type(int type, char *out);

SDL_Color get_pixel(SDL_Surface *surf, int x, int y);
Uint32 get_pixel_int(SDL_Surface *surf, int x, int y);
void set_pixel(SDL_Surface *surface, int x, int y, Uint32 pixel);

int my_rand(int seed);
float my_rand_float(int seed);

int minimum(int a, int b);

float lerp(float a, float b, float t);
int clamp(int a, int min, int max);
float clampf(float a, float min, float max);

float distance(float ax, float ay, float bx, float by);

bool is_point_on_line(SDL_Point p, SDL_Point a, SDL_Point b);
bool is_point_in_rect(SDL_Point p, SDL_Rect r);
bool is_point_in_triangle(SDL_Point pt, SDL_Point v1, SDL_Point v2, SDL_Point v3);

SDL_Point closest_point_on_line(SDL_Point a, SDL_Point b, SDL_Point p);
void draw_text(TTF_Font *font, const char *str, SDL_Color col, int align_right, int align_bottom, int x, int y, int *out_w, int *out_h);

#endif  /* UTIL_H_ */
