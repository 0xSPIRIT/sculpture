// A wrapper for calls to SDL's draw functions.
// We use this for things like global offsets,
// and to manage render targets better.

#define RENDERAPI static inline

#define MAX_TEXT_LENGTH 512

#define RenderTarget(which) (gs->render.render_targets+which)

typedef enum {
    VIEW_STATE_UNDEFINED,   // Invalid
    VIEW_STATE_SCREENSPACE, // Size: gs->window_width, gs->window_height
    VIEW_STATE_PIXELS,      // Size: gs->gw, gs->gh
} View_State;

typedef struct {
    SDL_Texture *handle;
    int width, height;
} Texture;

typedef struct {
    Texture texture;        // Contains the full size of the texture = full_width, full_height
    int working_width;      // Whereas the working size parameters are less,
    int working_height;     // enabling the use of negative texture positions.
    SDL_Point top_left;
    View_State view; // Is this in screenspace or pixel art?
} Render_Target;

typedef struct {
    TTF_Font *handle;
    int char_width, char_height;
} Font;

typedef enum {
    ALIGNMENT_TOP_LEFT,
    ALIGNMENT_TOP_RIGHT,
    ALIGNMENT_BOTTOM_LEFT,
    ALIGNMENT_BOTTOM_RIGHT,
    ALIGNMENT_CENTER
} Alignment;

typedef enum {
    TEXT_RENDER_BLENDED,
    TEXT_RENDER_LCD,
    TEXT_RENDER_SOLID
} Text_Render_Type;

// The structure used when drawing text to the screen.
typedef struct {
    char identifier[64]; // An identifier to keep track of this draw.
                         // Used so RenderDrawText will automatically
                         // not redraw if no changes are seen to the
                         // text in subsequent calls.
    Font *font;
    char str[MAX_TEXT_LENGTH];
    int x, y;
    SDL_Color foreground;
    SDL_Color background;
    Alignment alignment;
    Text_Render_Type render_type;
    bool force_update;
    Uint8 alpha;
    
    f64 game_scale;       // OUT: The scale of the game at the time of rendering.
    Texture texture;      // OUT: The texture generated
    SDL_Surface *surface; // OUT: The surface generated
} Render_Text_Data;

typedef struct {
    Render_Text_Data data[128];
    int size;
} Render_Text_Data_Cache;

typedef struct {
    SDL_Renderer *sdl;
    View_State view_type;
    
    SDL_FRect view;
    SDL_FPoint to;
    
    Render_Target *current_target;
    Render_Target *render_targets;
    
    Render_Text_Data_Cache text_cache; // The permanent cache, destroyed upon RenderCleanup
    Render_Text_Data_Cache temp_text_cache; // Temporary cache, destroyed at end of frame.
    int unfreed_objects;
} Render;

RENDERAPI Render RenderInit(SDL_Renderer *sdl_renderer);
RENDERAPI void RenderCleanup(Render *render);

static Render_Target *RenderMaybeSwitchToTarget(int target_enum);

RENDERAPI SDL_Surface *RenderLoadSurface(const char *fp);
RENDERAPI Texture RenderLoadTexture(const char *fp);
RENDERAPI void RenderDestroyTarget(Render_Target *target);
RENDERAPI void RenderDestroyTexture(Texture *tex);
RENDERAPI Texture RenderCreateTextureFromSurface(SDL_Surface *surf);
RENDERAPI Font *RenderLoadFont(const char *fp, int size);

RENDERAPI SDL_Rect RenderGetUpdatedRect(Render_Target *target, SDL_Rect *rect);
RENDERAPI Render_Target RenderMakeTarget(int width, int height, View_State view, bool use_negative_coords);
RENDERAPI Render_Target RenderMakeTargetEx(int width, int height, View_State view, bool use_negative_coords, bool streaming);
RENDERAPI void RenderTargetToTargetRelative(int target_dst, int target_src, SDL_Rect *src, SDL_Rect *dst);
RENDERAPI void RenderTargetToTargetRelativeEx(int target_dst, int target_src, SDL_Rect *src, SDL_Rect *dst, f64 angle, SDL_Point *center, SDL_RendererFlip flip);

RENDERAPI void RenderColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a);
RENDERAPI void RenderColorStruct(SDL_Color rgba);
RENDERAPI void RenderLine(int target, int x1, int y1, int x2, int y2);
RENDERAPI void RenderPointRelative(int target, int x1, int y1);
RENDERAPI void RenderDrawRectRelative(int target, SDL_Rect rect);
RENDERAPI void RenderFillRectRelative(int target, SDL_Rect rect);
RENDERAPI void RenderClear(int target);
RENDERAPI void RenderPresent(int target);

RENDERAPI void RenderTexture(int target, Texture *texture, SDL_Rect *src, SDL_Rect *dst);
RENDERAPI void RenderTextureEx(int target, Texture *texture, SDL_Rect *src, SDL_Rect *dst, f64 angle, SDL_Point *center, SDL_RendererFlip flip);

RENDERAPI void RenderTextureRelative(int target, Texture *texture, SDL_Rect *src, SDL_Rect *dst);

RENDERAPI void RenderDrawText(int target, Render_Text_Data *text_data);
RENDERAPI void RenderDrawTextQuick(int target_enum, const char *identifier, Font *font, const char *str, SDL_Color color, Uint8 alpha, int x, int y, int *w, int *h, bool force_redraw);
RENDERAPI void RenderCleanupTextCache(Render_Text_Data_Cache *cache);
RENDERAPI void RenderSetFontSize(Font *font, int size);
RENDERAPI void RenderReadPixels(int target, Uint8 *pixels, int pitch);
RENDERAPI void RenderTextureColorMod(Texture *texture, Uint8 r, Uint8 g, Uint8 b);
RENDERAPI void RenderTextureAlphaMod(Texture *texture, Uint8 a);
RENDERAPI int  RenderLockTexture(Texture *texture, SDL_Rect *rect, void **pixels, int *pitch);
RENDERAPI void RenderUnlockTexture(Texture *texture);

RENDERAPI void RenderFillCircle(int target, int x, int y, int size);