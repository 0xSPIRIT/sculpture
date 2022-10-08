#define MOUSE_BUTTONS 16

struct Input {
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
};
