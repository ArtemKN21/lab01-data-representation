
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>

int value = 100;

int main(void) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        value = 200;
        printf("Child: PID = %d, value = %d, address = %p\n",
               getpid(), value, (void *)&value);
    } else {
        value = 300;
        printf("Parent: PID = %d, value = %d, address = %p\n",
               getpid(), value, (void *)&value);
    }

    return 0;
}
