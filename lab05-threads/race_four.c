#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define THREAD_COUNT 4
#define ITERATIONS 500000

long counter = 0;

void *increment(void *arg)
{
    (void)arg;

    for (long i = 0; i < ITERATIONS; i++) {
        counter++;
    }

    return NULL;
}

int main(void)
{
    pthread_t threads[THREAD_COUNT];

    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_create(&threads[i], NULL, increment, NULL);
    }

    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Expected: %ld\n",
           (long)THREAD_COUNT * ITERATIONS);

    printf("Actual:   %ld\n",
           counter);

    return EXIT_SUCCESS;
}
