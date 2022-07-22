#include "vectorizer.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "util.h"

static int next_pixel(struct Vectorizer *v, int x, int y) {
    if (x < 0 || y < 0 || y >= v->w || y >= v->h) return 0;
    if (v->bitmap[x+y*v->w] != 1) return 0;
    
    v->bitmap[x+y*v->w] = 2; // Mark it as completed.
    
    v->line_count++;
    if (!v->line_start) {
        v->line_start = calloc(1, sizeof(struct Line));
        v->line_start->x = x;
        v->line_start->y = y;
        v->line_start->next = v->line_start->prev = NULL;
        v->line_head = v->line_start;
    } else {
        struct Line *p = v->line_head;
        v->line_head = calloc(1, sizeof(struct Line));
        v->line_head->x = x; 
        v->line_head->y = y;
        v->line_head->next = NULL;
        v->line_head->prev = p;
        p->next = v->line_head;
    }

    // Order seems to be important, so I won't bother putting this into a loop.
    next_pixel(v, x+1, y);
    next_pixel(v, x+1, y+1);
    next_pixel(v, x-1, y);
    next_pixel(v, x-1, y-1);
    next_pixel(v, x, y+1);
    next_pixel(v, x-1, y+1);
    next_pixel(v, x, y-1);
    next_pixel(v, x+1, y-1);

    return 1;
}

/* Takes the list of lines which go between each pixel and simplifies
 * them as much as possible by removing consecutive lines with the same
 * angle.
 */
static void simplify_vector(struct Vectorizer *v) {
    int c = 0;
    int initial = 1;
    
    for (struct Line *line = v->line_head; line != v->line_head || initial; line = line->next) {
        // If we are at least 3 lines in...
        if (!initial && line->prev != v->line_head && line->prev->prev != v->line_head) {
            // Find angle between current & previous
            float t1 = atan2(line->y - line->prev->y, line->x - line->prev->x);
            // And the current & previous' previous
            float t2 = atan2(line->y - line->prev->prev->y, line->x - line->prev->prev->x);
                
            // If the angles are the same there's no need to have two lines for the same angle, delete the middle one.
            if (t1 == t2) {
                struct Line *double_previous = line->prev->prev;
                free(line->prev);
                double_previous->next = line;
                line->prev = double_previous;
                v->line_count--;
            }
        }
        initial = 0;
        c++;
    }        
}

/* vectorize() converts a bitmap image to a linked list of lines.
 * Takes in pointer to 1D array.
 */
struct Vectorizer *vectorize(int *bitmap, int w, int h) {
    struct Vectorizer *v = calloc(1, sizeof(struct Vectorizer));
    v->w = w;
    v->h = h;
    v->bitmap = calloc(w*h, sizeof(int));
    memcpy(v->bitmap, bitmap, sizeof(int)*w*h);

    make_outline(v->bitmap, w, h);

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            if (v->bitmap[x+y*w]) {
                next_pixel(v, x, y);
                break;
            }
        }
    }
    // Loop the end to the start.
    v->line_head->next = v->line_start;
    v->line_start->prev = v->line_head;

    simplify_vector(v);

    return v;
}

// Modifies the bitmap to only its outline.
void make_outline(int *bitmap, int w, int h) {
    int *output = calloc(w*h, sizeof(int));
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            if (!bitmap[x+y*w]) continue;
            // Check if any neighbours are blank cells.
            // Don't check diagonals!
            int amt_blank = 0;
            for (int xx = -1; xx <= 1; xx++) {
                for (int yy = -1; yy <= 1; yy++) {
                    if (xx == yy || xx+yy == 0) continue;
                    if (bitmap[(xx+x)+(yy+y)*w] < 1) {
                        amt_blank = 1;
                        break;
                    }
                }
            }
            if (amt_blank) {
                output[x+y*w] = 1;
            }
        }
    }
    memcpy(bitmap, output, sizeof(int)*w*h);
    free(output);
}

void free_vectorizer(struct Vectorizer *v) {
    free(v->bitmap);
    struct Line *next;
    for (struct Line *line = v->line_start; line; line = next) {
        next = line->next;
        if (line->prev) line->prev->next = NULL;
        free(line);
    }
    free(v);
}

struct Line *vectorizer_find_intersecting_line(struct Vectorizer *v, int x, int y) {
    int initial = 1;
    for (struct Line *line = v->line_head; line != v->line_head || initial; line = line->next) {
        if (is_point_on_line((SDL_Point){x, y}, (SDL_Point){line->x, line->y}, (SDL_Point){line->next->x, line->next->y})) {
            return line;
            break;
        }
        initial = 0;
    }
    return NULL;
}
