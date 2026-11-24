#include <stdio.h>
#include <stdlib.h>

#define N 1000000L

int main(void)
{
    long sum = 0;

    for (long i = 1; i <= N; i++) {
        sum += i;
    }

    long expected = N * (N + 1) / 2;

    printf("Sequential result: %ld\n", sum);
    printf("Expected result:   %ld\n", expected);
    printf("Correct: %s\n", sum == expected ? "YES" : "NO");

    return EXIT_SUCCESS;
}
