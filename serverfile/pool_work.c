#include "chat.h"

void task(void *arg)
{
    pool_t *argv = (pool_t *)arg;
    struct sockaddr_in hisend;
    socklen_t len;
    int ret, res, sd, fd;
    char buf[BUFSISE];
    len = sizeof(hisend);

    hisend.sin_family = AF_INET;
    hisend.sin_port = htons(argv->port);
    inet_pton(AF_INET, argv->ipaddr, &hisend.sin_addr);

    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd == -1) {
        perror("socket");
        exit(1);
    }

    ret = connect(sd, (struct sockaddr *)&hisend, len);
    if (ret == -1) {
        perror("connect");
        exit(1);
    }

    fd = open(argv->path, O_RDONLY);
    if (fd == -1) {
        perror("open");
        exit(1);
    }

    while(1) {
        ret = read(fd, buf, BUFSISE);
        if (ret == -1) {
            perror("read");
            exit(1);
        }

        if (ret == 0) {
            usleep(100000);
            close(fd);
            close(sd);
            break;

        }

        res = write(sd, buf, ret);
        if (res == -1) {
            perror("write");
            exit(1);
        }

    }
    free(argv);
    return;
}

int pool_work(pool_t *buf)
{
    threadPoll_assign_work(task,(void *)buf, 10);
    return 0;
}
