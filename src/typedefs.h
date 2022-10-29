#define export __declspec(dllexport)
#define allocator __declspec(allocator)

#define Radians(x) ((x) * (M_PI / 180.0))
#define Degrees(x) ((x) * (180.0 / M_PI))

#define Log(...) printf(__VA_ARGS__), fflush(stdout)
#define Error(...) fprintf(stderr, __VA_ARGS__), fflush(stderr)

typedef float f32;
typedef double f64;
