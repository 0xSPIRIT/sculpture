#define BLOCKER_MAX_POINTS 256

enum Blocker_State {
    BLOCKER_STATE_OFF,
    BLOCKER_STATE_LINE,
    BLOCKER_STATE_CURVE,
};

struct Blocker {
    enum Blocker_State state, prev_state;
    struct SDL_Point points[BLOCKER_MAX_POINTS];
    int point_count;

    struct Line lines[2]; // The lines that extend to the ends.

    f64 angle;
    Uint32 *pixels;
};
