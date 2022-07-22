#include "globals.h"

SDL_Window *window;
SDL_Renderer *renderer;

SDL_Texture *render_tex;

SDL_Cursor *grabber_cursor, *normal_cursor;

int S = 7;
int window_width = 128*7, window_height = 128*7;

int mx, my;
int pmx, pmy;
int real_mx, real_my;
Uint32 mouse;

Uint8 *keys;

struct Vectorizer *v = 0;

float delta_time = 0.f;

int current_tool = TOOL_GRABBER;

TTF_Font *font, *title_font;
