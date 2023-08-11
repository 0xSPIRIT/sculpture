#define MOUSE_BUTTONS 16
#define MOUSE_SIMULATED false // Use simulated mouse for web?

typedef struct Input {
    int s_mx, s_my, s_pmx, s_pmy; // Used when MOUSE_SIMULATED == true

    int mx, my; // Scaled to the pixel-art grid.
    int real_mx, real_my; // In real window coordinates
    int pmx, pmy;
    int real_pmx, real_pmy;
    Uint32 mouse;
    Uint8 *keys;

    Uint8 keys_pressed[SDL_NUM_SCANCODES];
    Uint8 keys_released[SDL_NUM_SCANCODES];

    Uint8 mouse_pressed[MOUSE_BUTTONS];
    Uint8 mouse_released[MOUSE_BUTTONS];

    Uint8 keys_previous[SDL_NUM_SCANCODES];
    Uint32 mouse_previous;
} Input;
