
#include <stdio.h>
#include <unistd.h>

int main(void) {
    printf("Message from printf\n");

    const char message[] = "Message from write\n";
    write(STDOUT_FILENO, message, sizeof(message) - 1);

    return 0;
}
