#include <stdio.h>
#include <stdint.h>

void print_binary(uint8_t value)
{
    for (int i = 7; i >= 0; i--) {
        printf("%u", (value >> i) & 1u);
    }
}

int main(void)
{
    int input;
    printf("Enter value (0-255): ");
    if (scanf("%d", &input) != 1 || input < 0 || input > 255) {
        printf("Invalid input. Please enter a number between 0 and 255.\n");
        return 1;
    }

    uint8_t val = (uint8_t)input;

    printf("\n--- 8-bit Data Inspection ---\n");
    printf("Decimal:      %u\n", val);
    printf("Hexadecimal:  0x%02X\n", val);
    printf("Binary:       ");
    print_binary(val);
    printf("\n");
    
    if (val >= 32 && val <= 126) {
        printf("Character:    %c\n", (char)val);
    } else {
        printf("Character:    (non-printable)\n");
    }

    printf("\n--- 32-bit Memory & Endianness Inspection ---\n");
    uint32_t val32 = 0x12345678;
    unsigned char *p = (unsigned char *)&val32;

    printf("Value: 0x%08X\n", val32);
    printf("Size:  %zu bytes\n\n", sizeof(val32));

    printf("Memory:\n");
    for (int i = 0; i < 4; i++) {
        printf("byte %d (addr %p): 0x%02X\n", i, (void *)&p[i], p[i]);
    }

    printf("\nByte order: ");
    if (p[0] == 0x78) {
        printf("Little-endian\n");
    } else if (p[0] == 0x12) {
        printf("Big-endian\n");
    } else {
        printf("Unknown/Mixed-endian\n");
    }

    return 0;
}
