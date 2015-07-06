#include "chat.h"

static void get_path(char *buf_read, char *path)
{
    char *tmp = buf_read + 9;
    while(1) {
        if (*tmp == ' ') {
            break;
        }

        tmp++;
    }

    while(1) {
        if (*tmp != ' ') {
            break;
        }

        tmp++;
    }

    memset(path, 0, BUFSISE);
    strncpy(path, tmp, strlen(tmp));
    return;
}

static void get_hisaddr_hisport(const char *buf, char *hisaddr, char *hisport)
{
    memset(hisaddr, 0, BUFSISE);
    memset(hisport, 0, BUFSISE);
    while(1) {
        if (*buf != ' ') {
            break;
        }
        buf++;
    }    

    while(1) {
        if (*buf == ' ') {
            *hisaddr = '\0';
            break;
        }
        *hisaddr++ = *buf++;

    }

    while(1) {
        if (*buf != ' ') {
            break;
        }

        buf++;
    }

    while(1) {
        if (*buf == ' ') {
            *hisport = '\0';
            break;
        }
        *hisport++ = *buf++; 
    }
    return;
}

static void socket_stream_init(struct sockaddr_in *hisend, socklen_t len, int * sd, char *hisaddr, char * hisport)
{
    int ret;

    hisend->sin_family = AF_INET;
    hisend->sin_port = htons(atoi(hisport));
    inet_pton(AF_INET, hisaddr, &hisend->sin_addr);

    *sd = socket(AF_INET, SOCK_STREAM, 0);
    if (*sd == -1) {
        perror("socket");
        exit(1);
    }

    ret = connect(*sd, (struct sockaddr *)hisend, len);
    if (ret == -1) {
        perror("connect");
        exit(1);
    }

}

int send_file_t(int sd, struct sockaddr_in *serverend, socklen_t len, char *buf_read, char *name)
{
    int ret, res, size, i = 0;
    chat_t  tmp;
    chat_t *buf_receive;
    chat_t *buf_send;
    char tmp1[BUFSISE];
    char tmp2[BUFSISE];
    char tmp3[BUFSISE];
    char path[BUFSISE];
    char hisaddr[BUFSISE];
    char hisport[BUFSISE];
    printf("wait");
    fflush(stdout);
    while(1) {
        ret = recvfrom(sd, &tmp, sizeof(chat_t), MSG_PEEK | MSG_DONTWAIT, NULL, NULL);
        if (ret == -1) {
            if (errno == EAGAIN) {
                usleep(250000);
                printf(".");
                fflush(stdout);
                i++;
                if (i < 100) {
                    continue;
                }
            }
        }

        if (i >= 100) {
            sprintf(tmp1, "overtime cancle %s", buf_read);
            size = sizeof(chat_t) + strlen(tmp1) + strlen(name);
            buf_send = (chat_t *)malloc(size);
            if (buf_send == NULL) {
                perror("malloc");
                exit(1);
            }

            memset(buf_send, 0, size);

            buf_send->packetlen = size;
            buf_send->login_or_register = LOGINSUCCESS;
            buf_send->click = OFF;
            strncpy(buf_send->name.buf, name, strlen(name));
            strncpy(buf_send->information.buf, tmp1, strlen(tmp1));
            ret = sendto(sd, buf_send, size, 0, (struct sockaddr *)serverend, len);
            if (ret == -1) {
                perror("sendto");
                exit(1);
            }

            break;
        }

        size = tmp.packetlen;
        buf_receive = (chat_t *)malloc(size);
        if (buf_receive == NULL) {
            perror("malloc");
            exit(1);
        }
        memset(buf_receive, 0, size);
        memset(tmp2, 0, BUFSISE);

        ret = recvfrom(sd, buf_receive, size, 0, NULL, NULL);
        if (ret == -1) {
            perror("recvfrom");
            exit(1);
        }

        if (strncmp(buf_receive->information.buf, "nofile", strlen("nofile")) == 0) {
            return -1;

        }

        sprintf(tmp2, "rxfile %s", name);
        if (strncmp(buf_receive->information.buf, tmp2, strlen(tmp2)) == 0) {
            get_path(buf_read, path);
            get_hisaddr_hisport((const char *)buf_receive->information.buf + strlen(tmp2), hisaddr, hisport);

            struct sockaddr_in hisend;
            socklen_t stream_len;
            int stream_sd, fd;
            stream_len = sizeof(hisend);
            socket_stream_init(&hisend, stream_len, &stream_sd, hisaddr, hisport);

            fd = open(path, O_RDONLY);
            if (fd == -1) {
                perror("open");
                exit(1);
            }

            while(1) {
                ret = read(fd, tmp3, BUFSISE);
                if (ret == -1) {
                    perror("read");
                    exit(1);
                }

                if (ret == 0) {
                    usleep(100000);
                    close(fd);
                    close(stream_sd);

                    return 0;
                }

                res = write(stream_sd, tmp3, ret);
                if (res == -1) {
                    perror("write");
                    exit(1);
                }

                printf(".");
                fflush(stdout);


            }

        }

        else {

            information_save(buf_receive, name);
            information_save(buf_receive, (const char *)name);
            printf("%s\n", buf_receive->information.buf);
            fflush(stdout);

        }
    }

    return 1;
}

static void get_hisname(char *information, char *hisname)
{
    char *tmp;
    tmp = strstr(information, "from ");
    if (tmp == NULL) {
        fprintf(stderr, "no from\n");
        exit(1);
    }

    memset(hisname, 0, BUFSISE);
    tmp = tmp + 5;

    strncpy(hisname, tmp, strlen(tmp));
    return;
}

static void socket_stream_init_recv(struct sockaddr_in *myend, socklen_t len, int * sd, char *myaddr, int myport) 
{
    int ret;

    myend->sin_family = AF_INET;
    myend->sin_port = htons(myport);
    inet_pton(AF_INET, myaddr, &myend->sin_addr);

    *sd = socket(AF_INET, SOCK_STREAM, 0);
    if (*sd == -1) {
        perror("socket");
        exit(1);
    }


    ret = bind(*sd, (struct sockaddr *)myend, len);
    if (ret == -1) {
        perror("22bind");
        exit(1);
    }


}

void get_save_path(char *buf, char *save_path)
{
    memset(save_path, 0, BUFSISE);
    buf = buf + 6;
    while(1) {
        if (*buf != ' ') {
            break;
        }
        buf++;
    }

    strncpy(save_path, buf, strlen(buf));
    return;
}

static void get_file_name(char *buf, char *file, char *myname)
{
    char *p;
    memset(file, 0, BUFSISE);
    p = buf + 6;
    while(1) {
        if (*p != ' ') {
            break;
        }

        p++;
    }

    p = p + strlen(myname);

    while(1) {
        if (*p == ' ') {
            *file = '\0';
            break;
        }

        *file++ = *p++;
    }
    return;

}
int receive_file_t(int sd, struct sockaddr_in *serverend, socklen_t len, char *information, char *name) 
{
    char hisname[BUFSISE];
    char buf[BUFSISE];
    char tmp[BUFSISE];
    char tmp1[BUFSISE];
    char file[BUFSISE];
    char save_path[BUFSISE];
    chat_t *buf_send;
    int size, ret, fd;

    printf("input nofile refuse file, input rxfile receive file\n");
    fflush(stdout);

    memset(tmp, 0, BUFSISE);
    ret = read(0, tmp, BUFSISE);
    if (ret == -1) {
        perror("read");
        exit(1);
    }

    tmp[ret-1] = '\0';
    if (strncmp(tmp, "rxfile", strlen("rxfile")) == 0) {
        get_save_path(tmp, save_path);
    }
    get_file_name(information, file,name);

    get_hisname(information, hisname);

    if (strncmp(tmp, "nofile", strlen("nofile")) == 0) {
        sprintf(tmp1, "nofile %s receive %s", hisname, file);
        size = sizeof(chat_t) + strlen(buf) + strlen(name);
        buf_send = (chat_t *)malloc(size);
        if (buf_send == NULL) {
            perror("malloc");
            exit(1);
        }

        memset(buf_send, 0, size);

        buf_send->packetlen = size;
        buf_send->login_or_register = LOGINSUCCESS;
        buf_send->click = OFF;
        strncpy(buf_send->name.buf, name, strlen(name));
        strncpy(buf_send->information.buf, tmp1, strlen(tmp1));
        ret = sendto(sd, buf_send, size, 0, (struct sockaddr *)serverend, len);
        if (ret == -1) {
            perror("sendto");
            exit(1);
        }

    }


    if (strncmp(tmp, "rxfile", strlen("rxfile")) == 0) {

        struct sockaddr_in myend, hisend;
        socklen_t mylen;

        int mysd, newsd, res;
        mylen = sizeof(myend);

        socket_stream_init_recv(&myend, mylen, &mysd, CLIENTIPADDR, CLIENTPORT);
        memset(tmp1, 0, BUFSISE );

        sprintf(tmp1, "rxfile %s %s %d", hisname, CLIENTIPADDR, CLIENTPORT);
        size = sizeof(chat_t) + strlen(tmp1) + strlen(name);
        buf_send = (chat_t *)malloc(size);
        if (buf_send == NULL) {
            perror("malloc");
            exit(1);
        }

        memset(buf_send, 0, size);

        buf_send->packetlen = size;
        buf_send->login_or_register = LOGINSUCCESS;
        buf_send->click = OFF;
        strncpy(buf_send->name.buf, name, strlen(name));
        strncpy(buf_send->information.buf, tmp1, strlen(tmp1));
        ret = sendto(sd, buf_send, size, 0, (struct sockaddr *)serverend, len);
        if (ret == -1) {
            perror("sendto");
            exit(1);
        }

        listen(mysd, 10);
        newsd = accept(mysd, (struct sockaddr *)&hisend, &len);
        fd = open(save_path, O_CREAT | O_WRONLY | O_TRUNC, 0755);
        if (fd == -1) {
            perror("open");
            exit(1);
        }
        while(1) {
            ret = read(newsd, tmp, BUFSISE);
            if (ret == -1) {
                perror("read");
                exit(1);
            }
            printf(".");
            fflush(stdout);

            if (ret == 0) {
                printf("receive finish\n");
                close(newsd);
                close(fd);
                break;
            }

            res = write(fd, tmp, ret);
            if (res == -1) {
                perror("write");
                exit(1);
            }

        }

    }

    return;

}

static void *pthread_handler(void *argv)
{
    pthread_detach(pthread_self());
    char command[BUFSISE];
    memset(command, 0, BUFSISE);
    sprintf(command, "mpg123 listenmusic/mine.mp3");
    system(command);
    return NULL;
}

static void creat_pthread_listen_music(char *path)
{
    pthread_t tid;
    int ret;


    ret = pthread_create(&tid, NULL, pthread_handler, path);
    if (ret) {
        fprintf(stderr, "pthread_create:%s\n", strerror(ret));
        exit(1);
    }

    usleep(100000);

    return;
}

void get_song_name(char *information, char *songname)
{
    char *p;
    memset(songname, 0, BUFSISE);
    p = information + 11;
    while(1) {
        if (*p != ' ') {
            break;
        }

        p++;
    }

    strncpy(songname, p, strlen(p));
    return;
}
int listen_music(int sd, struct sockaddr_in *serverend, socklen_t len, chat_t *buf) 
{
    int ret, res;
    char path[BUFSISE];
    char tmp[BUFSISE];
    char songname[BUFSISE];
    struct sockaddr_in myend, hisend;
    socklen_t mylen;
    int tcpsd, fd, newsd;
    int size = buf->packetlen;
    mylen = sizeof(myend);

    get_song_name(buf->information.buf, songname);
    memset(path, 0, BUFSISE);
    sprintf(path, "listenmusic/%s", songname);
    socket_stream_init_recv(&myend, mylen, &tcpsd, buf->clientipaddr, buf->clientport);
    ret = sendto(sd, buf, size, 0, (struct sockaddr *)serverend, len);
    if (ret == -1) {
        perror("sendto");
        exit(1);
    }


    listen(tcpsd, 10);
    newsd = accept(tcpsd, (struct sockaddr *)&hisend, &len);
    if (newsd == -1) {
        perror("accept");
        exit(1);
    }
    fd = open("listenmusic/mine.mp3", O_CREAT | O_WRONLY | O_TRUNC, 0755);
    if (fd == -1) {
        perror("open");
        exit(1);
    }

    printf("load");
    fflush(stdout);

    while(1) {
        memset(tmp, 0, BUFSISE);
        ret = read(newsd, tmp, BUFSISE);
        if (ret == -1) {
            perror("read");
            exit(1);
        }
        printf(".");
        fflush(stdout);

        if (ret == 0) {
            close(newsd);
            close(tcpsd);
            close(fd);
            creat_pthread_listen_music(path);
            return 0;
        }

        res = write(fd, tmp, ret);
        if (res == -1) {
            perror("write");
            exit(1);
        }

    }
    return 0;
}

