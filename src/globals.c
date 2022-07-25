#include "globals.h"

SDL_Window *window;
SDL_Renderer *renderer;

SDL_Texture *render_tex;

SDL_Cursor *grabber_cursor, *normal_cursor;

int S = 6;
int window_width = 128*6, window_height = 128*6+GUI_H;

int mx, my;
int pmx, pmy;
int real_mx, real_my;
Uint32 mouse;

Uint8 *keys;

float delta_time = 0.f;

int current_tool = TOOL_GRABBER;

TTF_Font *font, *title_font;
