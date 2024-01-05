/*

    This is an implementation of a memory allocator 

*/
#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>



struct block_meta {
    /*
        metadata for a chunk of memory
    */
   size_t size;
   struct block_meta *next;
   int free;
};

#define META_SIZE sizeof(struct block_meta)

// The head of the memory block linked list
void *global_base = NULL;


struct block_meta *find_free_block(struct block_meta **last, size_t size)
/*
    look for a free memory block that is big enough
*/
{
    struct block_meta *current = global_base;
    while (current && !(current->free && current->size >= size)) {
        *last = current; 
        current = current->next;
    }
    return current; 
}


struct block_meta *request_space(struct block_meta* last, size_t size)
/*
    Request memory from the operating system, and create a new memory block with it
*/
{
    struct block_meta *block;
    block = sbrk(0);
    void *request = sbrk(size + META_SIZE);
    assert((void*)block == request); 
    if (request == (void*) -1) {
        return NULL; //sbrk failed to allocate memory
    }
    if (last) {
        last->next = block;
    }

    block->size = size;
    block->next = NULL;
    block->free = 0;
    return block;
}

void *cameron_malloc(size_t size) 
{
    struct block_meta *block;

    if (size <= 0) {
        return NULL;
    }

    if (!global_base) { // first call to malloc
        block = request_space(NULL, size);
        if (!block) {
            return NULL;
        }
        global_base = block; 
    } else {
        struct block_meta *last = global_base;
        block = find_free_block(&last, size);
        if (!block) { // no sufficient blocks available
            block = request_space(last, size);
            if (!block) {
                return NULL;
            }
        } else {
            block->free = 0;
        }
    }
    return(block+1);
}

struct block_meta *get_block_ptr(void *ptr)
/*
    take a pointer to a block of memory, and return a pointer to its block meta
*/
{
    return (struct block_meta*)ptr - 1;
}

void cameron_free(void *ptr) 
{
    if (!ptr) {
        return;
    }
    struct block_meta* block_ptr = get_block_ptr(ptr);
    assert(block_ptr->free == 0);
    block_ptr->free = 1;
}