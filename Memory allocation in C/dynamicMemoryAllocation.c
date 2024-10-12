#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

#define METADATA_SIZE 20
#define ALIGN_4(x) ((((x - 1) >> 2) << 2) + 4)

typedef struct memory_block* block_ptr;

struct memory_block {
    int is_free;
    size_t block_size;
    block_ptr next_block;
    block_ptr prev_block;
    void* memory_ptr;
    char memory[1];
};

void* heap_start = NULL;

// Helper function to find a suitable free block
block_ptr find_free_block(block_ptr* last_block, size_t size) {
    block_ptr current = heap_start;
    while (current && !(current->is_free && current->block_size >= size)) {
        *last_block = current;
        current = current->next_block;
    }
    return current;
}

// Helper function to split a block if it's too large
void split_block(block_ptr block, size_t size) {
    block_ptr new_block = (block_ptr)(block->memory + size);
    new_block->block_size = block->block_size - size - METADATA_SIZE;
    new_block->next_block = block->next_block;
    new_block->is_free = 1;
    new_block->memory_ptr = new_block->memory;
    new_block->prev_block = block;
    block->next_block = new_block;
    block->block_size = size;
    if (new_block->next_block) {
        new_block->next_block->prev_block = new_block;
    }
}

// Helper function to extend the heap
block_ptr grow_heap(block_ptr last_block, size_t size) {
    block_ptr new_block = sbrk(0);
    if (sbrk(METADATA_SIZE + size) == (void*)-1) {
        return NULL;
    }
    new_block->block_size = size;
    new_block->is_free = 0;
    new_block->next_block = NULL;
    new_block->prev_block = last_block;
    new_block->memory_ptr = new_block->memory;
    if (last_block) {
        last_block->next_block = new_block;
    }
    return new_block;
}

// Custom malloc implementation
void* custom_malloc(size_t size) {
    block_ptr block, last;
    size_t aligned_size = ALIGN_4(size);

    if (heap_start) {
        last = heap_start;
        block = find_free_block(&last, aligned_size);
        if (block) {
            if (block->block_size - aligned_size >= (METADATA_SIZE + 4)) {
                split_block(block, aligned_size);
            }
            block->is_free = 0;
        } else {
            block = grow_heap(last, aligned_size);
            if (!block) {
                return NULL;
            }
        }
    } else {
        block = grow_heap(NULL, aligned_size);
        if (!block) {
            return NULL;
        }
        heap_start = block;
    }
    return block->memory;
}

// Helper function to merge adjacent free blocks
block_ptr coalesce_blocks(block_ptr block) {
    if (block->next_block && block->next_block->is_free) {
        block->block_size += METADATA_SIZE + block->next_block->block_size;
        block->next_block = block->next_block->next_block;
    }
    if (block->next_block) {
        block->next_block->prev_block = block;
    }
    return block;
}

// Helper function to get the block address from a pointer
block_ptr get_block_from_ptr(void* ptr) {
    return (block_ptr)((char*)ptr - METADATA_SIZE);
}

// Helper function to validate a memory address
int is_valid_address(void* ptr) {
    if (heap_start) {
        if (ptr > heap_start && ptr < sbrk(0)) {
            return ptr == get_block_from_ptr(ptr)->memory_ptr;
        }
    }
    return 0;
}

// Custom free implementation
void custom_free(void* ptr) {
    if (is_valid_address(ptr)) {
        block_ptr block = get_block_from_ptr(ptr);
        block->is_free = 1;
        if (block->prev_block && block->prev_block->is_free) {
            block = coalesce_blocks(block->prev_block);
        }
        if (block->next_block) {
            coalesce_blocks(block);
        } else {
            if (block->prev_block) {
                block->prev_block->next_block = NULL;
            } else {
                heap_start = NULL;
            }
            brk(block);
        }
    }
}

// Helper function to copy data between blocks
void copy_block_data(block_ptr src, block_ptr dest) {
    size_t *src_data = (size_t*)src->memory;
    size_t *dest_data = (size_t*)dest->memory;
    size_t i;
    for (i = 0; i * sizeof(size_t) < src->block_size && i * sizeof(size_t) < dest->block_size; i++) {
        dest_data[i] = src_data[i];
    }
}

// Custom realloc implementation
void* custom_realloc(void* ptr, size_t size) {
    size_t aligned_size;
    block_ptr block, new_block;
    void* new_ptr;

    if (!ptr) {
        return custom_malloc(size);
    }

    if (is_valid_address(ptr)) {
        aligned_size = ALIGN_4(size);
        block = get_block_from_ptr(ptr);

        if (block->block_size >= aligned_size) {
            if (block->block_size - aligned_size >= (METADATA_SIZE + 4)) {
                split_block(block, aligned_size);
            }
        } else {
            if (block->next_block && block->next_block->is_free &&
                (block->block_size + block->next_block->block_size + METADATA_SIZE) >= aligned_size) {
                coalesce_blocks(block);
                if (block->block_size - aligned_size >= (METADATA_SIZE + 4)) {
                    split_block(block, aligned_size);
                }
            } else {
                new_ptr = custom_malloc(aligned_size);
                if (!new_ptr) {
                    return NULL;
                }
                new_block = get_block_from_ptr(new_ptr);
                copy_block_data(block, new_block);
                custom_free(ptr);
                return new_ptr;
            }
        }
        return ptr;
    }
    return NULL;
}

// Custom calloc implementation
void* custom_calloc(size_t num, size_t size) {
    size_t total_size = num * size;
    void* ptr = custom_malloc(total_size);

    if (ptr) {
        size_t aligned_size = ALIGN_4(total_size);
        size_t* zero_ptr = (size_t*)ptr;
        for (size_t i = 0; i < aligned_size / sizeof(size_t); i++) {
            zero_ptr[i] = 0;
        }
    }

    return ptr;
}