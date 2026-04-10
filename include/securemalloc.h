#ifndef SECUREMALLOC_H
#define SECUREMALLOC_H

#include <stddef.h>
#include <stdint.h>

/* Canary value to detect buffer overflows */
#define CANARY_VALUE  0xDEADBEEFCAFEBABE
#define POISON_ALLOC  0xCD
#define POISON_FREE   0xDD

/* Block header — sits before every allocation */
typedef struct BlockHeader {
    size_t size;
    uint64_t canary_start;
    int is_free;
    struct BlockHeader *next;
    struct BlockHeader *prev;
} BlockHeader;

/* Block footer — sits after every allocation */
typedef struct BlockFooter {
    uint64_t canary_end;
    size_t size;
} BlockFooter;

/* Allocation record for leak tracking */
typedef struct AllocRecord {
    void* address;
    size_t size;
    int line;
    const char* file;
    struct AllocRecord* next;
} AllocRecord;

/* Public API */
void* secure_malloc(size_t size);
void  secure_free(void* ptr);
void* secure_realloc(void* ptr, size_t size);
void  secure_check(void* ptr);
void  secure_scan(void* ptr, size_t size);
void  secure_dump();
void  secure_leak_report();

/* Macro to track file and line number automatically */
#define SECURE_MALLOC(size) secure_malloc_tracked(size, __FILE__, __LINE__)
#define SECURE_FREE(ptr)    secure_free(ptr)

void* secure_malloc_tracked(size_t size, const char* file, int line);

#endif