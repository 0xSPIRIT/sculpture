typedef struct {
    int text_width;
    bool stop;
    bool clicked_yet;
    Effect effect;
    Preview preview;
} Titlescreen;

#ifdef ALASKA_RELEASE_MODE
  #define SHOW_TITLESCREEN 1
#else
  #define SHOW_TITLESCREEN 0
#endif
