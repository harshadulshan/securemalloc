#include <stdio.h>
#include "../include/securemalloc.h"
#include "../src/db_logger.h"

int main() {
    printf("=== SecureMalloc Layer 4 — Database Logger ===\n\n");

    /* Initialize database */
    db_init("D:\\traning project\\securemalloc\\securemalloc.db");

    /* Test 1 — allocate and log */
    printf("-- Test 1: Allocate and log --\n");
    int*  a = (int*)SECURE_MALLOC(sizeof(int) * 10);
    char* b = (char*)SECURE_MALLOC(128);
    db_log_alloc(a, sizeof(int) * 10, __FILE__, __LINE__);
    db_log_alloc(b, 128,              __FILE__, __LINE__);

    /* Test 2 — free and update db */
    printf("\n-- Test 2: Free and update --\n");
    SECURE_FREE(a);
    db_log_free(a);

    /* b is intentionally not freed — simulates leak */

    /* Test 3 — report */
    db_report();

    /* Test 4 — leak report from memory */
    secure_leak_report();

    db_close();

    printf("=== Layer 4 Tests Done ===\n");
    return 0;
}