#define MOUSE_BUTTONS 16
#ifdef __EMSCRIPTEN__
  #define SIMULATE_MOUSE 1
#else
  #define SIMULATE_MOUSE 0
#endif

typedef struct Input {
    bool initted;
    bool locked;
    bool hide_mouse; // Only used when locked==true and we're drawing our own cursor.
    
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