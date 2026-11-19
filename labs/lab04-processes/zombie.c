
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>

int main(void) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        printf("Child process exiting. PID = %d\n", getpid());
        exit(0);
    }

    printf("Parent process PID = %d\n", getpid());
    printf("Child PID = %d\n", pid);
    printf("Parent sleeping for 60 seconds without wait().\n");
    fflush(stdout);

    sleep(60);

    return 0;
}
