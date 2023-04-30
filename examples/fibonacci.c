#include <stdio.h>

int main() {
    int a = 0;
    int b = 1;

    int steps = 30;

    for (int i = 1; i <= steps; i++) {
        int tmp = a + b;
        a = b;
        b = tmp;

        printf("%2d: %10d\n", i, a);
    }
    return 0;
}
