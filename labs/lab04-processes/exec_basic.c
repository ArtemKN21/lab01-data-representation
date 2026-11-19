
#include <stdio.h>
#include <unistd.h>

int main(void) {
    printf("Before exec: PID = %d\n", getpid());
    fflush(stdout);

    execlp("echo", "echo", "Hello from exec!", (char *)NULL);

    perror("execlp");
    return 1;
}
