#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "../include/securemalloc.h"

/* ------------------------------------------------
   LAYER 1 — Custom Allocator
   LAYER 2 — Security (canary, poison, double free)
   LAYER 3 — Leak tracking
   ------------------------------------------------ */

static BlockHeader*  heap_start    = NULL;
static AllocRecord*  alloc_records = NULL;

/* ----------- internal helpers ----------- */

static BlockFooter* get_footer(BlockHeader* header) {
    char* ptr = (char*)(header + 1);
    return (BlockFooter*)(ptr + header->size);
}

static void set_canaries(BlockHeader* header) {
    header->canary_start = CANARY_VALUE;
    BlockFooter* footer  = get_footer(header);
    footer->canary_end   = CANARY_VALUE;
    footer->size         = header->size;
}

static int check_canaries(BlockHeader* header) {
    if (header->canary_start != CANARY_VALUE) {
        printf("[SECUREMALLOC] OVERFLOW DETECTED before block!\n");
        printf("[SECUREMALLOC] Address: %p\n", (void*)header);
        return 0;
    }
    BlockFooter* footer = get_footer(header);
    if (footer->canary_end != CANARY_VALUE) {
        printf("[SECUREMALLOC] OVERFLOW DETECTED after block!\n");
        printf("[SECUREMALLOC] Address: %p\n", (void*)header);
        return 0;
    }
    return 1;
}

/* ----------- leak tracker ----------- */

static void record_alloc(void* addr, size_t size, const char* file, int line) {
    AllocRecord* rec = (AllocRecord*)malloc(sizeof(AllocRecord));
    rec->address = addr;
    rec->size    = size;
    rec->line    = line;
    rec->file    = file;
    rec->next    = alloc_records;
    alloc_records = rec;
}

static void remove_record(void* addr) {
    AllocRecord* cur  = alloc_records;
    AllocRecord* prev = NULL;
    while (cur != NULL) {
        if (cur->address == addr) {
            if (prev) prev->next = cur->next;
            else alloc_records   = cur->next;
            free(cur);
            return;
        }
        prev = cur;
        cur  = cur->next;
    }
}

/* ----------- public API ----------- */

void* secure_malloc(size_t size) {
    if (size == 0) return NULL;

    size_t total         = sizeof(BlockHeader) + size + sizeof(BlockFooter);
    BlockHeader* header  = (BlockHeader*)malloc(total);

    if (header == NULL) {
        printf("[SECUREMALLOC] ERROR: System out of memory!\n");
        return NULL;
    }

    header->size    = size;
    header->is_free = 0;
    header->next    = NULL;
    header->prev    = NULL;

    set_canaries(header);
    memset(header + 1, POISON_ALLOC, size);

    if (heap_start == NULL) {
        heap_start = header;
    } else {
        BlockHeader* cur = heap_start;
        while (cur->next != NULL) cur = cur->next;
        cur->next    = header;
        header->prev = cur;
    }

    printf("[SECUREMALLOC] Allocated %zu bytes at %p\n", size, (void*)(header + 1));
    return (void*)(header + 1);
}

void* secure_malloc_tracked(size_t size, const char* file, int line) {
    void* ptr = secure_malloc(size);
    if (ptr) record_alloc(ptr, size, file, line);
    return ptr;
}

void secure_free(void* ptr) {
    if (ptr == NULL) {
        printf("[SECUREMALLOC] WARNING: free(NULL) called!\n");
        return;
    }

    BlockHeader* header = ((BlockHeader*)ptr) - 1;

    if (!check_canaries(header)) {
        printf("[SECUREMALLOC] DANGER: Corrupted block — aborting free!\n");
        return;
    }

    if (header->is_free) {
        printf("[SECUREMALLOC] DANGER: Double free detected at %p!\n", ptr);
        return;
    }

    memset(ptr, POISON_FREE, header->size);
    header->is_free = 1;

    if (header->prev) header->prev->next = header->next;
    if (header->next) header->next->prev = header->prev;
    if (heap_start == header) heap_start  = header->next;

    remove_record(ptr);
    free(header);
    printf("[SECUREMALLOC] Freed block at %p\n", ptr);
}

void secure_check(void* ptr) {
    if (ptr == NULL) return;
    BlockHeader* header = ((BlockHeader*)ptr) - 1;
    if (check_canaries(header))
        printf("[SECUREMALLOC] Block at %p is SAFE\n", ptr);
}

void secure_scan(void* ptr, size_t size) {
    unsigned char* bytes = (unsigned char*)ptr;
    for (size_t i = 0; i < size; i++) {
        if (bytes[i] == POISON_FREE) {
            printf("[SECUREMALLOC] USE-AFTER-FREE detected at offset %zu in block %p\n", i, ptr);
            return;
        }
    }
    printf("[SECUREMALLOC] Block %p looks clean\n", ptr);
}

void secure_dump() {
    printf("\n--- SECUREMALLOC HEAP DUMP ---\n");
    BlockHeader* cur = heap_start;
    int count = 0;
    while (cur != NULL) {
        printf("Block %d | Address: %p | Size: %zu | Status: %s\n",
            count, (void*)(cur + 1), cur->size,
            cur->is_free ? "FREE" : "LIVE");
        check_canaries(cur);
        cur = cur->next;
        count++;
    }
    if (count == 0) printf("Heap is empty.\n");
    printf("------------------------------\n\n");
}

void secure_leak_report() {
    printf("\n=== MEMORY LEAK REPORT ===\n");
    AllocRecord* cur = alloc_records;
    int    count = 0;
    size_t total = 0;
    while (cur != NULL) {
        printf("LEAK | Address: %p | Size: %zu bytes | File: %s | Line: %d\n",
            cur->address, cur->size, cur->file, cur->line);
        total += cur->size;
        count++;
        cur = cur->next;
    }
    if (count == 0)
        printf("No leaks detected. All memory freed correctly.\n");
    else
        printf("Total leaks: %d blocks | %zu bytes lost\n", count, total);
    printf("==========================\n\n");
}