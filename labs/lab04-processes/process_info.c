
#include <stdio.h>
#include <unistd.h>

int main(void) {
    printf("Process ID (PID): %d\n", getpid());
    printf("Parent Process ID (PPID): %d\n", getppid());

    printf("Process is running for 30 seconds...\n");
    sleep(30);

    printf("Process finished.\n");

    return 0;
}
