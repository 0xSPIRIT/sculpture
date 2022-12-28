#define MAX_LINES 100

struct Narrator {
    char lines[MAX_LINES][128];
    int line_curr, line_count;
};
