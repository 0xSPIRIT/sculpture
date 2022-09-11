#ifndef POPUP_H
#define POPUP_H

#include <stdbool.h>
#include <SDL2/SDL.h>

struct Text_Field {
    char description[256];
    char text[256];
    bool active;

    void (*on_return)(const char *);
};

void set_text_field(const char *description, const char *initial_text, void (*on_return)(const char *));
void text_field_tick(SDL_Event *event);
void text_field_draw();

#endif // POPUP_H
