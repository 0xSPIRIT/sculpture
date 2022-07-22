#ifndef UTIL_H_
#define UTIL_H_

#include <SDL2/SDL.h>

void get_name_from_type(int type, char *out);
SDL_Color get_pixel(SDL_Surface *surf, int x, int y);
void set_pixel(SDL_Surface *surface, int x, int y, Uint32 pixel);
int my_rand(int seed);
int min(int a, int b);
int max(int a, int b);
int clamp(int a, int min, int max);
float clampf(float a, float min, float max);
float distance(SDL_Point a, SDL_Point b);
int is_point_on_line(SDL_Point p, SDL_Point a, SDL_Point b);
int is_point_in_rect(SDL_Point p, SDL_Rect r);
SDL_Point closest_point_on_line(SDL_Point a, SDL_Point b, SDL_Point p);

#endif  /* UTIL_H_ */
