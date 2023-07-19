#define MAX_DUST_COUNT 64*64

typedef struct Dust {
    enum Cell_Type type;
    f64 x, y;
    f64 vx, vy;
    int timer, timer2;
    int timer_max;
    bool going_into_inventory;
    int rand;
} Dust;