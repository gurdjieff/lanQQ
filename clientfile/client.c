#include "chat.h"


int inet_init(struct sockaddr_in *myend, struct sockaddr_in *serverend, int sd, socklen_t len)
{
    int ret;
    myend->sin_family = AF_INET;
    myend->sin_port = htons(CLIENTPORT);
    inet_pton(AF_INET, CLIENTIPADDR, &myend->sin_addr);
    serverend->sin_family = AF_INET;
    serverend->sin_port = htons(SERVERPORT);
    inet_pton(AF_INET, SERVERIPADDR, &serverend->sin_addr);

    sd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sd == -1) {
        perror("socket");
        exit(1);
    }

    ret = bind(sd, (struct sockaddr *)myend, len);
    if (ret == -1) {
        perror("bind");
        exit(1);
    }

    return sd;

}

void malloc_t(chat_t **buf, int size)
{

    *buf = (chat_t *)malloc(size);
    if (*buf == NULL) {
        perror("malloc");
        exit(1);
    }

    memset(*buf, 0, size);
    return;

}
void poll_init(struct pollfd *fds, int sd)
{
    fds[0].fd = sd;
    fds[0].events = POLLIN;
    fds[1].fd = 0;
    fds[1].events = POLLIN;

    return;
}

int client_init(int sd, chat_t *tmp,  struct sockaddr_in *serverend, socklen_t len)
{
    chat_t *buf;
    int ret, size, res;
    if (tmp == NULL) {
        fprintf(stderr, "error\n");
        exit(1);
    }

    ret = sendto(sd, tmp, tmp->packetlen, 0, (struct sockaddr *)serverend, len);
    if (ret == -1) {
        perror("sendto");
        exit(1);
    }

    ret = recvfrom(sd, tmp, tmp->packetlen, MSG_PEEK, NULL, NULL);
    if (ret == -1) {
        perror("recvfrom");
        exit(1);
    }

    size = tmp->packetlen;
    malloc_t(&buf, size);

    ret = recvfrom(sd, buf, size, 0, NULL, NULL);
    if (ret == -1) {
        perror("recvfrom");
        exit(1);
    }

    res = buf->res;
    free(buf);

    return res;

}

int register_and_login(void)
{
    int ret;
    char buf[BUFSISE];
    while(1) {


        printf("please input ,l is login, input new is register a new account\n");
        memset(buf, 0, BUFSISE);
        ret = read(1, buf, BUFSISE);
        if (ret == -1) {
            perror("ret");
            exit(1);
        }

        buf[ret-1] = '\0';
        if (strcmp(buf, "new") == 0) {
            return 2;
        }

        if (strcmp(buf, "l") == 0) {
            return 1;
        }

        else {
            printf("input wrong\n");
            continue;
        }
    }
}

int login_input(int sd, struct sockaddr_in *serverend, socklen_t len, char *name)
{
    int ret1, ret2, i, size, ret;
    chat_t * tmp;
    char buf1[BUFSISE];
    char buf2[BUFSISE];
    i = 0;

    while(1) {
        printf("please input ID\n");
        ret1 = read(1, buf1, BUFSISE);
        if (ret1 == -1) {
            perror("read");
            exit(1);
        }
        buf1[ret1-1] = '\0';

        printf("please input passwd\n");
        ret2 = read(1, buf2, BUFSISE);
        if (ret2 == -1) {
            perror("read");
            exit(1);
        }
        buf2[ret2-1] = '\0';


        size = sizeof(chat_t) + ret1 + ret2;
        malloc_t(&tmp, size);

        strncpy(tmp->name.buf, buf1, ret1);
        strncpy(tmp->passwd.buf, buf2, ret2);
        tmp->packetlen = size;
        tmp->login_or_register = LOGIN;
        tmp->click = OFF;
        strncpy(tmp->clientipaddr, CLIENTIPADDR, strlen(CLIENTIPADDR));
        tmp->clientport = CLIENTPORT;

        strncpy(name, tmp->name.buf, strlen(tmp->name.buf));
        ret = client_init(sd, tmp, serverend, len);

        if (ret == -1) {
            printf("username or passwd is wrong, please input again\n");
            i++;
            if (i > 3) {
                printf("username or passwd too much wrong, exit\n");
                exit(0);
            }
            continue;
        }
        if (ret == 1) {
            printf("login success\n");
            break;
        }
    }

    free(tmp);
    return 0;

}

int register_input(int sd, struct sockaddr_in * serverend, socklen_t len)
{
    chat_t *tmp;
    int ret1, ret2, size, ret;
    char buf1[BUFSISE];
    char buf2[BUFSISE];

    while(1) {
        printf("please input ID\n");
        ret1 = read(1, buf1, BUFSISE);
        if (ret1 == -1) {
            perror("ret");
            exit(1);
        }

        buf1[ret1-1] = '\0';

        printf("please input passwd\n");
        ret2 = read(1, buf2, BUFSISE);
        if (ret2 == -1) {
            perror("ret");
            exit(1);
        }

        buf2[ret2-1] = '\0';
        size = sizeof(chat_t) + ret1 + ret2;

        malloc_t(&tmp, size);

        strncpy(tmp->name.buf, buf1, ret1);
        strncpy(tmp->passwd.buf, buf2, ret2);
        tmp->packetlen = size;
        tmp->login_or_register = REGISTER;
        tmp->click = OFF;

        ret = client_init(sd, tmp, serverend, len);
        if (ret == -2) {
            printf("user is exist, please input again\n");
            continue;
        }

        if (ret == 2) {
            printf("register success, please login\n");
            break;
        }
    }
    free(tmp);
    return 0;

}

static void *pthread_handler(void *argv)
{
    int size, ret;
    click_str *p = (click_str *)argv;
    struct sockaddr_in *serverend = p->serverend;
    int sd = p->sd;
    socklen_t len = p->len;

    char *name = p->name;
    chat_t *buf;
    size = sizeof(chat_t) + strlen(name);

    pthread_detach(pthread_self());
    malloc_t(&buf, size);
    buf->packetlen = size;
    strncpy(buf->name.buf, name, strlen(name));
    buf->login_or_register = LOGINSUCCESS;
    buf->click = ON;

    while(1) {
        ret = sendto(sd, buf, size, 0, (struct sockaddr *)serverend, len);
        sleep(2);
    }

    return NULL;

}

void click_work(int sd, struct sockaddr_in *serverend, socklen_t len, char *name)
{

    click_str *argv;
    int size;
    size = sizeof(click_str) + strlen(name);
    argv = malloc(size);
    if (argv == NULL) {
        perror("malloc");
        exit(1);
    }
    memset(argv, 0, size);
    argv->sd = sd;
    argv->serverend = serverend;
    argv->len = len;
    strncpy(argv->name, name, strlen(name));
    int ret;
    pthread_t tid;
    ret = pthread_create(&tid, NULL, pthread_handler, argv);
    if(ret) {
        fprintf(stderr, "create: %s\n", strerror(ret));
        exit(1);
    }

    return;
}

void get_name_information(char *buf, char *name, char *information)
{
    char *p, *q;
    p = buf + 4;
    q = name;
    while(1) {
        if (*p != ' ') {
            break;
        }

        p++;
    }

    while(1) {

        if (*p == ' ') {
            *name = '\0';
            break;
        }

        *name++ = *p++;
    }

    while(1) {
        if (*p != ' ') {
            break;
        }

        p++;
    }

    strncpy(information, p, strlen(p));

    return;


}

void information_save(chat_t *buf, const char *file)
{   
    if (strncmp(buf->information.buf, "chat", strlen("chat")) == 0 || strncmp(buf->information.buf, "from", strlen("from")) == 0) {

        int fd, ret;
        time_t cur;
        struct tm * res;
        off_t ft;
        char tmp[BUFSISE];
        char name[BUFSISE];
        char information[BUFSISE];
        char path[BUFSISE];

        memset(path, 0, BUFSISE);
        sprintf(path, "log/%s",file);

        time(&cur);

        res = localtime(&cur);
        if (res == NULL) {
            perror("localtime");
            exit(1);
        }

        fd = open(path, O_CREAT | O_WRONLY, 0755);
        if (fd == -1) {
            perror("open");
            exit(1);
        }

        ft = lseek(fd, 0, SEEK_END);
        if (ft == -1) {
            perror("lseek");
            exit(1);
        }

        if (strncmp(buf->information.buf, "chat", strlen("chat")) == 0) {
            get_name_information(buf->information.buf, name, information);
            sprintf(tmp, "%s   %d-%d-%d %d:%d %s %s\n", "To", res->tm_year+1900, res->tm_mon+1, res->tm_mday, res->tm_hour, res->tm_min, name, information);

            ret = write(fd, tmp, strlen(tmp));
            if (ret == -1) {
                perror("write");
                exit(1);
            }

        }


        if (strncmp(buf->information.buf, "from", strlen("from")) == 0) {
            get_name_information(buf->information.buf + 1, name, information);
            sprintf(tmp, "%s %d-%d-%d %d:%d %s %s\n", "From", res->tm_year+1900, res->tm_mon+1, res->tm_mday, res->tm_hour, res->tm_min, name, information);

            ret = write(fd, tmp, strlen(tmp));
            if (ret == -1) {
                perror("write");
                exit(1);
            }

        }

    }
}

char * get_name_path(char * buf_recv, char * title)
{
    if (*buf_recv != '#' || *(buf_recv+1) != '#') {
        return NULL;
    }
    char *tmp;
    tmp = buf_recv + 2;
    while(1) {
        if (*tmp == '#') {
            *title = '\0';
            return tmp + 2;
        }
        *title++ = *buf_recv++;
    }
}

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

int client_work(int sd, struct sockaddr_in *serverend, socklen_t len, char *name)
{
    int ret, size;
    struct pollfd fds[2];
    chat_t *buf, tmp;
    char buf_read[BUFSISE];
    char buf_recv[BUFSISE];
    char buf_tmp[BUFSISE];
    char path[BUFSISE];

    click_work(sd, serverend, len, name);
    poll_init(fds, sd);

    while(1) {
        printf("liaoqq> ");
        fflush(stdout);
        ret = poll(fds, 2, -1);
        if (ret == -1) {
            perror("poll");
            exit(1);
        }


        if (fds[0].revents & POLLIN) {
            ret = recvfrom(sd, &tmp, sizeof(chat_t), MSG_PEEK, NULL, NULL);
            if (ret == -1) {
                perror("recvfrom");
                exit(1);
            }

            size = tmp.packetlen;
            malloc_t(&buf, size);
            ret = recvfrom(sd, buf, size, 0, NULL, NULL);
            if (ret == -1) {
                perror("recvfrom");
                exit(1);
            }

            if (strncmp(buf->information.buf, "sendfile", strlen("sendfile")) == 0) {
                printf("%s\n", buf->information.buf);
                ret = receive_file_t(sd, serverend, len, buf->information.buf, name);

                continue;
            }

            information_save(buf, (const char *)name);
            printf("%s\n", buf->information.buf);
            fflush(stdout);
            free(buf);
        }

        if (fds[1].revents & POLLIN) {
            memset(buf_read, 0, BUFSISE);
            ret = read(0, buf_read, BUFSISE);
            if (ret == -1) {
                perror("read");
                exit(1);
            }

            if (ret == 1) {
                continue;
            }

            buf_read[ret-1] = '\0';


            get_path(buf_read, path);

            if (strncmp(buf_read, "sendfile",strlen("sendfile")) == 0) {
                if (access(path, F_OK)) {
                    fprintf(stderr, "file is not exist\n");
                    continue;
                }
            }


            size = sizeof(chat_t) + ret + strlen(name);
            malloc_t(&buf, size);

            buf->packetlen = size;
            buf->login_or_register = LOGINSUCCESS;
            strncpy(buf->clientipaddr, CLIENTIPADDR, strlen(CLIENTIPADDR));
            buf->clientport = CLIENTPORT;
            buf->click = OFF;
            strncpy(buf->name.buf, name, strlen(name));
            strncpy(buf->information.buf, buf_read, strlen(buf_read));


            information_save(buf,(const char *)name);
            ret = sendto(sd, buf, size, 0, (struct sockaddr *)serverend, len);
            if (ret == -1) {
                perror("sendto");
                exit(1);
            }
            if (strncmp(buf_read, "listenmusic",strlen("listenmusic")) == 0 && strcmp(buf_read, "listenmusic") != 0) {
                ret = listen_music(sd, serverend, len, buf);
                continue;
            }
            else if (strncmp(buf_read, "sendfile",strlen("sendfile")) == 0) {
                ret = send_file_t(sd, serverend, len, buf_read, name);

                if (ret == 0) {
                    fprintf(stderr, "send success\n");
                    continue;
                }

                else if (ret == -1) {
                    fprintf(stderr, "sendfile was refuse\n");        
                    continue;
                }

                else if (ret == 1) {
                    fprintf(stderr, "overtime no reply\n");        
                    continue;
                }
            }


            if (strcmp(buf_read, "exit") == 0) {
                printf("Bye,Bye %s\n", name);
                exit(0);
            }

            free(buf);


        }
    }
}

int main(void)
{
    int sd, ret;
    char name[BUFSISE];
    struct sockaddr_in myend, serverend;
    socklen_t len;

    len = sizeof(myend);
    sd = inet_init(&myend, &serverend, sd, len);

    memset(name, 0, BUFSISE);
    ret = register_and_login();
    if (ret == 2) {
        register_input(sd, &serverend, len);
        login_input(sd, &serverend, len, name);
    }

    if (ret == 1) {
        login_input(sd, &serverend, len, name);
    }

    ret = client_work(sd, &serverend, len, name);
    return 0;

}
