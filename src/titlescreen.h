typedef struct {
    bool active;
    char text_input[64];
    bool focus;
    int caret_timer;
} Titlescreen_Goto_Level;

typedef struct {
    int text_width;
    bool stop;
    bool clicked_yet;
    Effect effect;
    Preview preview;

    bool fading;

    int state; // 0 = normal titlescreen, 1 = skip level screen

    Titlescreen_Goto_Level goto_level_field;
} Titlescreen;

#ifdef ALASKA_RELEASE_MODE
  #define SHOW_TITLESCREEN 1
#else
  #define SHOW_TITLESCREEN 1
#endif
