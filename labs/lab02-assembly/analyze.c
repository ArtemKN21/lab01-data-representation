#include <stdio.h>

int analyze(int a, int b)
{
    int result;

    if (a > b) {
        result = a - b;
    } else {
        result = a + b;
    }

    int sum = 0;

    for (int i = 1; i <= result; i++) {
        sum += i;
    }

    return sum;
}

int main(void)
{
    int result = analyze(5, 3);

    printf("Result: %d\n", result);

    return 0;
}
