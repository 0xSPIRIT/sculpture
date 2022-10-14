// I don't know how this works.
// ... Magic!
SDL_Point get_spline_point(SDL_Point *points, float t) {
    int p0, p1, p2, p3;

    p1 = (int)t + 1;
    p2 = p1 + 1;
    p3 = p2 + 1;
    p0 = p1 - 1;

    t = t - (int)t;
    
    float t2 = t*t;
    float t3 = t*t*t;

    float q1 = -t3 + 2.f*t2 - t;
    float q2 = 3.f*t3 - 5.f*t2 + 2.f;
    float q3 = -3.f*t3 + 4.f*t2 + t;
    float q4 = t3 - t2;

    float tx = points[p0].x * q1 + points[p1].x * q2 + points[p2].x * q3 + points[p3].x * q4;
    float ty = points[p0].y * q1 + points[p1].y * q2 + points[p2].y * q3 + points[p3].y * q4;

    return (SDL_Point){(int)tx, (int)ty};
}
