#define MAX_DUST_COUNT (gs->gw*gs->gh)

typedef struct Dust {
    Cell_Type type;
    f32 x, y;
    f32 vx, vy;
    int timer, timer2;
    int timer_max;
    bool going_into_inventory;
    bool destroyed_via_tool;
    int rand;
} Dust;
