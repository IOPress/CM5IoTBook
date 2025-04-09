#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>
#include <sched.h>
#include <unistd.h>

volatile int j;
volatile int i;

void * threadA(void *p) {
    for (i = 0;; i++) {
    };
}

void * threadB(void *p) {
    for (j = 0;; j++) {
    };
}

int main(int argc, char** argv) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(1, &cpuset);

    pthread_t pthreadA;
    pthread_create(&pthreadA, NULL, threadA, NULL);
    pthread_setaffinity_np(pthreadA, sizeof (cpu_set_t), &cpuset);

    CPU_ZERO(&cpuset);
    CPU_SET(2, &cpuset);
    pthread_t pthreadB;
    pthread_create(&pthreadB, NULL, threadB, NULL);
    pthread_setaffinity_np(pthreadB, sizeof (cpu_set_t), &cpuset);

    sleep(5);

    printf("%d %d", i, j);
    return (EXIT_SUCCESS);
}