#include <stdio.h>
#include <stdlib.h>

int main() {
    printf("SecureMalloc project ready!\n");
    printf("GCC is working!\n");

    /* Test basic memory */
    int *p = malloc(sizeof(int) * 10);
    if (p == NULL) {
        printf("Memory allocation failed!\n");
        return 1;
    }

    p[0] = 42;
    printf("Memory test passed! Value: %d\n", p[0]);
    free(p);
    printf("All systems ready!\n");

    return 0;
}