#include <stdio.h>
#include <string.h>
#include "../include/securemalloc.h"

int main() {
    printf("=== SecureMalloc Layer 3 — Leak Detection ===\n\n");

    /* Test 1 — allocate and free properly */
    printf("-- Test 1: Clean allocations --\n");
    int* a = (int*)SECURE_MALLOC(sizeof(int) * 10);
    char* b = (char*)SECURE_MALLOC(64);
    SECURE_FREE(a);
    SECURE_FREE(b);
    secure_leak_report();

    /* Test 2 — deliberate leak */
    printf("-- Test 2: Deliberate Leaks --\n");
    int* leak1 = (int*)SECURE_MALLOC(100);
    char* leak2 = (char*)SECURE_MALLOC(200);
    /* intentionally NOT freeing leak1 and leak2 */
    (void)leak1;
    (void)leak2;
    secure_leak_report();

    printf("=== Layer 3 Tests Done ===\n");
    return 0;
}