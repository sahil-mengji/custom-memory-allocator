# Custom Memory Allocator

## Overview

This project implements a custom memory allocator in C, providing functionality similar to the standard library's dynamic memory allocation functions. Dynamic memory allocation allows programs to request memory at runtime, which is crucial for efficient memory management and handling variable-sized data structures.

## Key Features

Our custom memory allocator includes implementations of the following functions:

- **custom_malloc**: Allocates a single block of memory of a specified size.
- **custom_calloc**: Allocates memory for an array of elements and initializes them to zero.
- **custom_realloc**: Resizes a previously allocated memory block.
- **custom_free**: Deallocates a previously allocated memory block.

## Function Descriptions

### `custom_malloc`

`void* custom_malloc(size_t size);`

`custom_malloc` allocates a single block of memory of the specified size and returns a pointer to the allocated block. If the allocation fails, it returns `NULL`.

### `custom_calloc`

`void* custom_calloc(size_t num, size_t size);`

`custom_calloc` allocates memory for an array of `num` elements, each of `size` bytes. It initializes all bytes in the allocated storage to zero and returns a pointer to the allocated memory. If the allocation fails, it returns `NULL`.

### `custom_realloc`

`void* custom_realloc(void* ptr, size_t size);`

`custom_realloc` changes the size of the memory block pointed to by `ptr` to `size` bytes. The contents of the memory block are preserved up to the lesser of the new and old sizes. If `ptr` is `NULL`, it behaves like `custom_malloc`. If `size` is zero and `ptr` is not `NULL`, it behaves like `custom_free`. If the allocation fails, it returns `NULL`.

### `custom_free`

`void custom_free(void* ptr);`

`custom_free` deallocates the memory block pointed to by `ptr`. If `ptr` is `NULL`, no operation is performed.

## Implementation Details

This custom memory allocator uses a linked list of memory blocks to manage allocations. It includes features such as:

- **Block splitting** to minimize wasted space
- **Block coalescing** to reduce fragmentation
- **Heap expansion** when necessary

The implementation aims to be efficient while providing a similar interface to the standard library functions.

## Usage

To use this custom memory allocator in your project:

1. Include the header file in your C program.
2. Replace calls to `malloc`, `calloc`, `realloc`, and `free` with their `custom_` counterparts.

### Example:

```c
#include "custom_allocator.h"

int main() {
    int* arr = (int*)custom_malloc(10 * sizeof(int));
    // Use the allocated memory
    custom_free(arr);
    return 0;
}
```
