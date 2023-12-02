typedef struct Pause_Menu {
    bool active;
    
    f32 slider; // 0 to 1
    bool holding_slider;
} Pause_Menu;

#define INITIAL_VOLUME 0.5