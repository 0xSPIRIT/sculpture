#define MOUSE_BUTTONS 16

typedef struct Input {
    int s_mx, s_my, s_pmx, s_pmy; // Used when MOUSE_SIMULATED == true

    int mx, my; // Scaled to the pixel-art grid.
    int real_mx, real_my; // In real window coordinates
    int pmx, pmy;
    int real_pmx, real_pmy;
    u32 mouse;
    u8 *keys;

    u8 keys_pressed[SDL_NUM_SCANCODES];
    u8 keys_released[SDL_NUM_SCANCODES];

    u8 mouse_pressed[MOUSE_BUTTONS];
    u8 mouse_released[MOUSE_BUTTONS];

    u8 keys_previous[SDL_NUM_SCANCODES];
    u32 mouse_previous;
} Input;
