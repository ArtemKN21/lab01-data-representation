
#include <stdio.h>
#include <unistd.h>

int main(void) {
    printf("Process PID: %d\n", getpid());
    printf("Process is sleeping for 60 seconds...\n");
    fflush(stdout);

    sleep(60);

    printf("Process finished.\n");

    return 0;
}
