#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include "Gpio5.h"
int main(int argc, char **argv)
{
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(3, &cpuset);
    int res = sched_setaffinity(getpid(), sizeof(cpu_set_t),&cpuset);
    rp1_Init();
    gpio_init(2);
    gpio_set_dir(2, true);
    while (1)
    {
        gpio_xor_mask(0x4);
    }
    return 0;
}