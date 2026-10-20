#include <stdio.h>
#include <stdint.h>

int main(void)
{
    int8_t signed_value = -1;
    uint8_t unsigned_value = 255;

    printf("Signed value (-1) hex:   0x%02X\n", (unsigned char)signed_value);
    printf("Unsigned value (255) hex: 0x%02X\n", unsigned_value);

    return 0;
}
