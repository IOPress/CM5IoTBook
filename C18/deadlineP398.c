#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <linux/sched.h>
#include <pthread.h>
#include <stdint.h>
#include "Gpio5.h"


struct sched_attr {
    uint32_t size;
    uint32_t sched_policy;
    uint64_t sched_flags;
    int32_t sched_nice;
    uint32_t sched_priority;
    uint64_t sched_runtime;
    uint64_t sched_deadline;
    uint64_t sched_period;
};

int sched_setattr(pid_t pid, const struct sched_attr *attr, 
        unsigned int flags) {
    return syscall(__NR_sched_setattr, pid, attr, flags);
}

void *threadA(void *p)
{
    struct sched_attr attr = {
        .size = sizeof(attr),
        .sched_policy = SCHED_DEADLINE,
        .sched_runtime = 10 * 1000,
        .sched_deadline = 25 * 1000,
        .sched_period = 100* 1000,
    };
    int result =sched_setattr(0, &attr, 0);
    for (;;)
    {
        gpio_xor_mask(0x4);
        sched_yield();
    };
}

int main(int argc, char **argv)
{
    rp1_Init();
    gpio_init(2);
    gpio_set_dir(2, true);
    pthread_t pthreadA;
    pthread_create(&pthreadA, NULL, threadA, NULL);
    pthread_exit(0);
    return (EXIT_SUCCESS);
}