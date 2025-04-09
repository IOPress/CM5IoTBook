#include <stdio.h>
#include <stdlib.h>
#include "Gpio5.h"
#include <sched.h> 

int main(int argc, char **argv)
{

    const struct sched_param priority = {1};
    sched_setscheduler(0, SCHED_FIFO, &priority);
    rp1_Init();
    gpio_init(2);
    gpio_set_dir(2, true);
    while (true)
    {
        gpio_put(2, 1);
        gpio_put(2, 0);
    }
    return (EXIT_SUCCESS);
}