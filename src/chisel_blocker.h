enum Chisel_Blocker_State {
    CHISEL_BLOCKER_OFF,
    CHISEL_BLOCKER_LINE_MODE,
    CHISEL_BLOCKER_CURVE_MODE,
};

struct Line {
    int x1, y1, x2, y2;
};

struct Chisel_Blocker {
    int state;
    // enum Chisel_Blocker_State state;
    
    SDL_Point control_points[4]; // The bezier or line control points.
    int point_count;

    struct Line lines[2]; // The lines that extend at the ends for the bezier.
    int current_point; // -1 for none
    int active; // Not to be confused with chisel_blocker_mode.

    Uint32 *pixels;

    int side;
};
