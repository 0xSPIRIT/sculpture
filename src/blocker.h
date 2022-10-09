#define BLOCKER_MAX_POINTS 512

struct Blocker {
    bool on;
    struct SDL_Point points[BLOCKER_MAX_POINTS];
    f64 angle;
    int point_count;

    Uint32 *pixels;
};
