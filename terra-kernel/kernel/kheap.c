#include <stdbool.h>
#include "headers/kheap.h"

#define MIN_CHUNK_SIZE sizeof(struct heapchunk_t) + 4

struct heapinfo_t {
    struct heapchunk_t *start;
    u64 avail;
};

struct heapchunk_t {
    u64 size;
    bool inuse;
    struct heapchunk_t *next;
}__attribute__((aligned(16)));

static struct heapinfo_t heap;

void heap_init(void) {
    heap.start = (struct heapchunk_t*)heap_start;
    heap.avail = (u64)heap_end - (u64)heap_start;

    heap.start->size = heap.avail - sizeof(struct heapchunk_t);
    heap.start->inuse = false;
    heap.start->next = NULL;
}

void* kalloc(size_t size) {
    size = (size + 15) & ~15;

    if (size > heap.avail) {
        return NULL;
    }

    struct heapchunk_t *chunk = heap.start;

    //check if the heap has enough space
    while (chunk != NULL) {
        if (!chunk->inuse && chunk->size >= size) {
            break;
        }
        chunk = chunk->next;
    }

    if (chunk == NULL) {
        return NULL;
    }

    size_t leftover = chunk->size - size;

    if (leftover > MIN_CHUNK_SIZE) {  // leave room for a new chunk
        struct heapchunk_t *new_chunk = (struct heapchunk_t *)((void *)chunk + sizeof(struct heapchunk_t) + size);
        new_chunk->size = leftover - sizeof(struct heapchunk_t);
        new_chunk->inuse = false;
        new_chunk->next = chunk->next;

        chunk->next = new_chunk;
        chunk->size = size;
    }

    chunk->inuse = true;

    return (void *)chunk + sizeof(struct heapchunk_t);
}

void coalesce_chunk(void) {
    struct heapchunk_t *chunk = heap.start;
    while (chunk && chunk->next) {
        if (!chunk->inuse && !chunk->next->inuse) {
            chunk->size += sizeof(struct heapchunk_t) + chunk->next->size;
            chunk->next = chunk->next->next;
        } else {
            chunk = chunk->next;
        }
    }
}

void kfree(void* ptr) {
    if (!ptr) return;
    if ((ptr < heap_start) | (ptr >= heap_end)) return;

    struct heapchunk_t *chunk = (struct heapchunk_t *)((void *)ptr - sizeof(struct heapchunk_t));

    if (!chunk->inuse) return;

    chunk->inuse = false;

    coalesce_chunk();
}

// assumes new_size is already 16-byte aligned
void* extendChunkInPlace(void* ptr, size_t new_size, struct heapchunk_t *old_chunk) {
    if (old_chunk->next->inuse == false) {
        size_t extra_space_required = new_size - old_chunk->size; //* get the space the chunk needs to be extended by

        if (old_chunk->next->size >= extra_space_required + sizeof(struct heapchunk_t)) { //* check if free chunk has enough space
            struct heapchunk_t* next = old_chunk->next;
            struct heapchunk_t* split = (struct heapchunk_t*)((void *)next + sizeof(struct heapchunk_t) + extra_space_required);

            split->size = next->size - extra_space_required - sizeof(struct heapchunk_t);
            split->inuse = false;
            split->next = next->next;

            old_chunk->size += extra_space_required;
            old_chunk->next = split;

            return (void *)old_chunk + sizeof(struct heapchunk_t);
        }
    }

    return NULL;
}

void* krealloc(void *ptr, size_t new_size) {
    new_size = (new_size + 15) & ~15;
    // if ptr == NULL kalloc() new block and return
    if (!ptr) {
        return kalloc(new_size);
    }

    struct heapchunk_t *old_chunk = (struct heapchunk_t *)((void *)ptr - sizeof(struct heapchunk_t));
    // if new size is larger the current size
    if (old_chunk->size < new_size) {

        // attempt to extend in place
        if (extendChunkInPlace(ptr, new_size, old_chunk)) {
            return ptr; // In-place extension successful
        }

        // otherwise allocate completely new block
        void* new_ptr = kalloc(new_size);
        if (!new_ptr) {
            return NULL;
        }
        memcpy(new_ptr, ptr, (old_chunk->size < new_size ? old_chunk->size : new_size));
        kfree(ptr); // free old block and coalesce heap

        return new_ptr; // return pointer to new block
    } else if (old_chunk->size > new_size) {

        // check if there is enough room to split chunk
        size_t leftover = old_chunk->size - new_size;
        if (leftover > MIN_CHUNK_SIZE) {
            struct heapchunk_t *new_chunk = (struct heapchunk_t *)((char *)old_chunk + sizeof(struct heapchunk_t) + new_size);
            new_chunk->size = leftover - sizeof(struct heapchunk_t);
            new_chunk->inuse = false;
            new_chunk->next = old_chunk->next;

            old_chunk->size -= leftover;
            old_chunk->next = new_chunk;
            coalesce_chunk();
            return (void*)old_chunk + sizeof(struct heapchunk_t);
        } else if (leftover < MIN_CHUNK_SIZE) {
            void* new_ptr = kalloc(new_size);
            if (!new_ptr) {
                return NULL;
            }
            
            memcpy(new_ptr, ptr, (old_chunk->size > new_size ? new_size : old_chunk->size));
            kfree(ptr); // free old block and coalesce heap

            return new_ptr; // return pointer to new block
        }
    }

    // if new_size == old_size do nothing
    return ptr;
}