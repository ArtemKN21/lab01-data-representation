
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>

int main(void) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        printf("Child process: PID = %d\n", getpid());
        fflush(stdout);

        execlp("echo", "echo", "Hello from child after exec!", (char *)NULL);

        perror("execlp");
        exit(1);
    }

    int status;
    waitpid(pid, &status, 0);

    printf("Parent: child process finished\n");

    return 0;
}
