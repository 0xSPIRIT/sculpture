#ifndef SHARED_H_
#define SHARED_H_ 

#include <stdint.h>
#include <stdio.h>

struct Game_Memory {
    uint8_t *ptr;
    uint8_t *cursor;
    size_t size;
};

// Allocate a block of memory of size 'size' in bytes.
inline void *game_allocate_memory(struct Game_Memory *memory, size_t size) {
    if (memory->cursor+size > memory->cursor+memory->size) {
        fprintf(stderr, "Out of memory!\n"); fflush(stderr);
        exit(1);
    }

    void *result = memory->cursor;
    memory->cursor += size;
    return result;
}

#endif
