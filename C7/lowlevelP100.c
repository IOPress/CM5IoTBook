#include <string.h>
#include <stdio.h> 
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/gpio.h>
#include <sys/epoll.h>

int main(int argc, char **argv) {
    int fd, ret;

    struct gpioevent_request req;
    req.lineoffset = 4;
    req.handleflags = GPIOHANDLE_REQUEST_INPUT;
    req.eventflags = GPIOEVENT_REQUEST_RISING_EDGE;
    strcpy(req.consumer_label, "Event test");

    fd = open("/dev/gpiochip0", O_RDONLY);
    ret = ioctl(fd, GPIO_GET_LINEEVENT_IOCTL, &req);
    close(fd);

    static struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = req.fd;
    int epfd = epoll_create(1);
    int res = epoll_ctl(epfd, EPOLL_CTL_ADD, req.fd, &ev);

    int nfds = epoll_wait(epfd, &ev, 1, 20000);
    if (nfds != 0) {
        struct gpioevent_data edata;
        read(req.fd, &edata, sizeof edata);
        printf("%u,%llu", edata.id, edata.timestamp);
    }
}