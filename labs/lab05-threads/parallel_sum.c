#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define N 1000000L
#define THREAD_COUNT 4

typedef struct {
    long start;
    long end;
    long result;
} Task;

void *calculate_sum(void *arg)
{
    Task *task = arg;

    long sum = 0;

    for (long i = task->start;
         i <= task->end;
         i++) {

        sum += i;
    }

    task->result = sum;

    return NULL;
}

int main(void)
{
    pthread_t threads[THREAD_COUNT];
    Task tasks[THREAD_COUNT];

    long chunk = N / THREAD_COUNT;

    for (int i = 0; i < THREAD_COUNT; i++) {
        tasks[i].start = i * chunk + 1;
        tasks[i].end = (i + 1) * chunk;
        tasks[i].result = 0;

        int result = pthread_create(
            &threads[i],
            NULL,
            calculate_sum,
            &tasks[i]
        );

        if (result != 0) {
            fprintf(stderr,
                    "pthread_create failed: %d\n",
                    result);
            return EXIT_FAILURE;
        }
    }

    long total = 0;

    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_join(threads[i], NULL);

        printf("Thread %d: %ld ... %ld = %ld\n",
               i + 1,
               tasks[i].start,
               tasks[i].end,
               tasks[i].result);

        total += tasks[i].result;
    }

    long expected = N * (N + 1) / 2;

    printf("\nParallel result: %ld\n", total);
    printf("Expected result: %ld\n", expected);
    printf("\nCorrect: %s\n",
           total == expected ? "YES" : "NO");

    return EXIT_SUCCESS;
}
