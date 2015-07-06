#include "chat.h"


int inet_init(struct sockaddr_in *serverend, int sd, socklen_t len)
{
    int ret;

    serverend->sin_family = AF_INET;
    serverend->sin_port = htons(SERVERPORT);
    inet_pton(AF_INET, SERVERIPADDR, &serverend->sin_addr);

    sd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sd == -1) {
        perror("socket");
        exit(1);
    }

    ret = bind(sd, (struct sockaddr *)serverend, len);
    if (ret == -1) {
        perror("bind");
    inet_pton(AF_INET, SERVERIPADDR, &serverend->sin_addr);
        exit(1);
    }

    return sd;
}


int server_register(int sd, chat_t *buf, struct sockaddr_in *user1end, socklen_t len)
{
    int ret, size, res;
    chat_t tmp;
    size = sizeof(chat_t);
    ret = chatregister((const chat_t *)buf);
    memset(&tmp, 0, size);
    tmp.packetlen = size;
    tmp.res = ret;
    res = ret;

    ret = sendto(sd, &tmp, size, 0, (struct sockaddr *)user1end, len);
    if (ret == -1) {
        perror("sendto");
        exit(1);
    }


    return res;

}

int server_login(int sd, chat_t *buf, struct sockaddr_in *user1end, socklen_t len, chat_t *head)
{
    int ret, size, res;
    chat_t tmp;
    chat_t *p;
    size = sizeof(chat_t);
    ret = login_t(buf);
    if (ret != -1) {
        p = list_find(head, buf->name.buf);
        if (p != NULL) {
            if (p->status != UNLINE) {
            ret = -1;
            }
        }
    }
    memset(&tmp, 0, size);
    res = ret;
    tmp.res = ret;

    tmp.packetlen = size;
    ret = sendto(sd, &tmp, size, 0, (struct sockaddr *)user1end, len);
    if (ret == -1) {
        perror("sendto");
        exit(1);
    }


    return res;

}

void send_getoff_information(int sd, getoff_information *argv, struct sockaddr_in *user1end, socklen_t len)
{
    if (argv == NULL) {
        return;
    }

    int size, ret;
    chat_t *buf;
    getoff_information *tmp, *freetmp;
    for (tmp = argv; tmp != NULL; tmp = tmp->next) {
        size = sizeof(chat_t) + strlen(tmp->buf);
        buf = (chat_t *)malloc(size);
        if (buf == NULL) {
            perror("malloc");
            exit(1);
        }

        memset(buf, 0, size);
        strncpy(buf->information.buf, tmp->buf, strlen(tmp->buf));
        buf->packetlen = size;
        ret = sendto(sd, buf, size, 0, (struct sockaddr *)user1end, len);
        if (ret == -1) {
            perror("sendto");
            exit(1);
        }

        free(buf);
    }


    tmp = argv;
    while(tmp != NULL) {
        freetmp = tmp;
        tmp = tmp ->next;
        if (tmp == NULL) {
            free(freetmp);
            break;
        }
        free(freetmp);
        
    }

    argv = NULL;

    return;

}

void login_success_init(int sd, chat_t *head, chat_t *buf, struct sockaddr_in *userend, socklen_t len)
{
    chat_t *tmp, *tmp2;

 //   userend->sin_family = AF_INET;
   // userend->sin_port = htons(buf->clientport);
   // inet_pton(AF_INET, buf->clientipaddr, &userend->sin_addr);

    tmp = list_find(head, buf->name.buf);
    tmp->status = ONLINE;
    memcpy(&tmp->userend, userend, sizeof(struct sockaddr_in));
    tmp->len = len;

    if (tmp->information.argv != NULL) {
        send_getoff_information(sd, tmp->information.argv, userend, len);
    }
    

    free(buf);

    return;
}


int main(void)
{
    int sd, ret, size;
    struct sockaddr_in userend, serverend;
    chat_t *buf, tmp;
    socklen_t len, userlen;
    chat_t *head;

    head = list_creat();
 //   display(head);
    pthread_create_work(head);
    threadPoll_create_and_init(10);

    len = sizeof(serverend);
    userlen = sizeof(userend);
    sd = inet_init(&serverend, sd, len);

    while(1) {
        ret = recvfrom(sd, &tmp, sizeof(chat_t), MSG_PEEK, (struct sockaddr *)&userend, &userlen);
        if (ret == -1) {
            perror("recvfrom");
            exit(1);
        }

        size = tmp.packetlen;

        buf = (chat_t *)malloc(size);
        if (buf == NULL) {
            perror("malloc");
            exit(1);
        }
        memset(buf, 0, size);

        
        ret = recvfrom(sd, buf, size, 0, NULL, NULL);

        if (ret == -1) {
            perror("recvfrom");
            exit(1);
        }



        if (buf->login_or_register == REGISTER) {
            ret = server_register(sd, buf, &userend, len);
            if (ret == 2) {
                node_creat_insert(head, buf->name.buf);
            }
        }

        if (buf->login_or_register == LOGIN) {
            ret = server_login(sd, buf, &userend, len, head);
            if (ret == 1) {
                login_success_init(sd, head, buf, &userend, userlen);
                fflush(stdout);
            }

        }

        if (buf->login_or_register == LOGINSUCCESS) {
            server_work(sd, head, buf);
        }
    }
    return 0;
}
