#include <stdio.h>

int calculate(int a, int b)
{
    return a + b;
}

int main(void)
{
    int value = calculate(10, 20);

    printf("%d\n", value);

    return 0;
}
