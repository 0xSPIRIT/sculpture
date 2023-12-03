#define MAX_WINDSTREAM_POINTS 64

typedef struct Wind_Stream {
    SDL_Point points[MAX_WINDSTREAM_POINTS];
    int point_count;
} Wind_Stream;

enum {
    WIND_STREAM_IDLE,
    WIND_STREAM_GROW,
    WIND_STREAM_SHRINK,
};

typedef struct Wind_Stream_Instance {
    Wind_Stream *stream;
    int range_start, range_end;
    int state; // 0 = idle, 1 = range_end++, 2 = range_start++
    bool active;
    int x, y;
} Wind_Stream_Instance;

typedef struct Wind {
    Wind_Stream streams[10];
    Wind_Stream_Instance instances[128];
    int instance_count;
    int timer;
} Wind;

void setup_winds();
void wind_streams_draw(int target, Wind *wind);
bool wind_stream_draw(int target, Wind_Stream_Instance *instance);