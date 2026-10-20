#include <stdio.h>

int main(void)
{
    printf("char:      %zu bytes, %zu bits\n", sizeof(char), sizeof(char) * 8);
    printf("short:     %zu bytes, %zu bits\n", sizeof(short), sizeof(short) * 8);
    printf("int:       %zu bytes, %zu bits\n", sizeof(int), sizeof(int) * 8);
    printf("long:      %zu bytes, %zu bits\n", sizeof(long), sizeof(long) * 8);
    printf("long long: %zu bytes, %zu bits\n", sizeof(long long), sizeof(long long) * 8);
    printf("float:     %zu bytes, %zu bits\n", sizeof(float), sizeof(float) * 8);
    printf("double:    %zu bytes, %zu bits\n", sizeof(double), sizeof(double) * 8);
    printf("void *:    %zu bytes, %zu bits\n", sizeof(void *), sizeof(void *) * 8);

    return 0;
}
