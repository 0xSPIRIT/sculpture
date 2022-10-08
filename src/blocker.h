#ifndef BLOCKER_H_
#define BLOCKER_H_

#define BLOCKER_MAX_POINTS 512

struct Blocker {
    bool on;
    struct SDL_Point points[BLOCKER_MAX_POINTS];
    int point_count;

    Uint32 *pixels;
};

#endif // BLOCKER_H_
