#ifndef VECTORIZER_H_
#define VECTORIZER_H_

struct Line {
    int x, y;
    struct Line *next, *prev;
};

struct Vectorizer {
    int *bitmap;
    int w, h;
    
    struct Line *line_start, *line_head; // Linked list of lines.
    int line_count;
};

struct Vectorizer *vectorize(int *bitmap, int w, int h);
void free_vectorizer(struct Vectorizer *v);
void make_outline(int *bitmap, int w, int h);

struct Line *vectorizer_find_intersecting_line(struct Vectorizer *v, int x, int y);

#endif  /* VECTORIZER_H_ */
