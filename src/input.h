#ifndef INPUT_H_
#define INPUT_H_

#include <SDL2/SDL.h>

#define MOUSE_BUTTONS 16

extern int mx, my; // Fake in game coordinates (pixel art scaled)
extern int real_mx, real_my; // In real window coordinates
extern int pmx, pmy;
extern int real_pmx, real_pmy;
extern Uint32 mouse;
extern Uint8 *keys;

extern Uint8 keys_pressed[SDL_NUM_SCANCODES];
extern Uint8 keys_released[SDL_NUM_SCANCODES];

extern Uint8 mouse_pressed[MOUSE_BUTTONS];
extern Uint8 mouse_released[MOUSE_BUTTONS];

void input_tick();

#endif  /* INPUT_H_ */
