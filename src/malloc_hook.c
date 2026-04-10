#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/securemalloc.h"
#include "../src/db_logger.h"

/* This program uses ONLY standard malloc/free
   but SecureMalloc will intercept everything */

int main() {
    printf("=== Malloc Hook Test ===\n\n");

    /* Initialize database */
    db_init("D:\\traning project\\securemalloc\\securemalloc.db");

    /* Standard malloc calls — no SECURE_MALLOC needed */
    printf("-- Allocating with standard malloc --\n");
    int*  nums = (int*)malloc(sizeof(int) * 5);
    char* name = (char*)malloc(64);
    void* data = calloc(10, sizeof(double));

    /* Use the memory */
    nums[0] = 100;
    nums[1] = 200;
    strcpy(name, "SecureMalloc Hook Test");

    printf("nums[0] = %d\n", nums[0]);
    printf("nums[1] = %d\n", nums[1]);
    printf("name    = %s\n", name);

    /* Log to database */
    db_log_alloc(nums, sizeof(int) * 5, __FILE__, __LINE__);
    db_log_alloc(name, 64,              __FILE__, __LINE__);
    db_log_alloc(data, 10*sizeof(double), __FILE__, __LINE__);

    /* Free some — simulate leak on data */
    printf("\n-- Freeing --\n");
    free(nums);
    db_log_free(nums);

    free(name);
    db_log_free(name);

    /* data intentionally not freed */

    /* Reports */
    secure_leak_report();
    db_report();
    db_close();

    printf("=== Hook Test Done ===\n");
    return 0;
}