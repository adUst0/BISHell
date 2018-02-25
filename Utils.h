#ifndef UTILS_H
#define UTILS_H

// Free the memory pointed by ptr and set it to NULL
#define delMemory(ptr)                                                 \
    free(ptr);                                                         \
    ptr = NULL;

// The program terminates with EXIT_FAILURE if there was a problem allocating memory.
#define assertAlloc(ptr)                                                \
    if(!ptr)                                                            \
    {                                                                   \
        write(2, "Allocation error!\n", strlen("Allocation error!\n")); \
        exit(EXIT_FAILURE);                                             \
    }

// Doubles the value of capacity and reallocates new memory for the ptr.
#define resizeArr(ptr, capacity)                                        \
    capacity *= 2;                                                      \
    ptr = realloc(ptr, sizeof(*ptr) * capacity);                        \
    assertAlloc(cmd->buff);

#endif //UTILS_H