#include <stdio.h>
#include <stdint.h>

int main(void)
{
    uint8_t vals[] = {254, 255};

    for (int i = 0; i < 2; i++) {
        uint8_t v = vals[i];
        printf("Initial: %3u -> ", v);
        v++;
        printf("After v++: %3u\n", v);
    }

    uint8_t value = 255;
    printf("255 + 1 = %u\n", (uint8_t)(value + 1));
    printf("255 + 2 = %u\n", (uint8_t)(value + 2));

    return 0;
}
