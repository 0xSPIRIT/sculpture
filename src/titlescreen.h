typedef struct Titlescreen {
    int text_width;
    bool stop;
    Effect effect;
    Preview preview;
} Titlescreen;

#ifdef ALASKA_RELEASE_MODE
  #define SHOW_TITLESCREEN 1
#else
  #define SHOW_TITLESCREEN 1
#endif
