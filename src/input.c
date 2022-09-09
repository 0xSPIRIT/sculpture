#include "input.h"

#include "globals.h"

int mx, my; // Fake in game coordinates (pixel art scaled)
int real_mx, real_my; // In real window coordinates
int pmx, pmy;
int real_pmx, real_pmy;

Uint32 mouse;
Uint8 *keys;

Uint8 keys_pressed[SDL_NUM_SCANCODES];
Uint8 keys_released[SDL_NUM_SCANCODES];

Uint8 mouse_pressed[MOUSE_BUTTONS];
Uint8 mouse_released[MOUSE_BUTTONS];

void input_tick() {
    pmx = mx;
    pmy = my;

    real_pmx = real_mx;
    real_pmy = real_my;

    static Uint8 keys_previous[SDL_NUM_SCANCODES] = {0};
    static Uint32 mouse_previous = {0};

    mouse = (Uint32) SDL_GetMouseState(&real_mx, &real_my);
    keys = (Uint8*) SDL_GetKeyboardState(NULL);

    for (int i = 0; i < SDL_NUM_SCANCODES; i++) {
        keys_pressed[i] = keys[i] && !keys_previous[i];
        keys_released[i] = !keys[i] && keys_previous[i];
    }
    for (int i = 0; i < MOUSE_BUTTONS; i++) {
        mouse_pressed[i] = mouse & SDL_BUTTON(i) && !(mouse_previous & SDL_BUTTON(i));
        mouse_released[i] = !(mouse & SDL_BUTTON(i)) && (mouse_previous & SDL_BUTTON(i));
    }

    mx = real_mx/S;
    my = real_my/S;
        
    my -= GUI_H/S;

    memcpy(keys_previous, keys, SDL_NUM_SCANCODES);
    mouse_previous = mouse;
}
