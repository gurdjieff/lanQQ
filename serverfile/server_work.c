#include "chat.h"

int order_list(chat_t *buf) 
{
    if (strncmp(buf->information.buf, "list", strlen("list")) == 0 ) {
        return 0;
    }

    else {
        return -1;
    }
}

void status_change(chat_t *tmp, char *status)

{
    if (tmp->status == ONLINE) {
        strncpy(status, "online", strlen("online"));
    }

    else if (tmp->status == UNLINE) {
        strncpy(status, "unline", strlen("unline"));
    }
    else if (tmp->status == GETOFF) {
        strncpy(status, "getoff", strlen("getoff"));
    }
    else if (tmp->status == HIDDEN) {
        strncpy(status, "hidden", strlen("hidden"));
    }
}

void display_list_excut(char *buf, chat_t *head)
{
    int ret, i = 0;
    chat_t *tmp;
    char status[BUFSISE];
    for (tmp = head->next; tmp != NULL; tmp = tmp->next) {
        if (tmp->status == GETOFF || tmp->status == ONLINE) {
            status_change(tmp, status);
            ret = sprintf(buf + i, "name :%s,  nickname: %s, status: %s\n",tmp->name.buf, tmp->nickName.buf, status);
            i = i + ret;
        }
    }

    return;
}

void display_list_all_excut(char *buf, chat_t *head)
{
    int ret, i = 0;
    chat_t *tmp;
    char status[BUFSISE];
    for (tmp = head->next; tmp != NULL; tmp = tmp->next) {
        status_change(tmp, status);
        ret = sprintf(buf + i, "name :%s,  nickname: %s, status: %s\n",tmp->name.buf, tmp->nickName.buf, status);
        i = i + ret;
    }

    return;
}

void display_list_hidden_excut(char *buf, chat_t *head)
{
    int ret, i = 0;
    chat_t *tmp;
    char status[BUFSISE];
    for (tmp = head->next; tmp != NULL; tmp = tmp->next) {
        if (tmp->status != HIDDEN) {
            continue;
        }
        status_change(tmp, status);
        ret = sprintf(buf + i, "name :%s,  nickname: %s, status: %s\n",tmp->name.buf, tmp->nickName.buf, status);
        i = i + ret;
    }

    return;
}
void send_to_client(int sd, chat_t *tmp, char *buf, int size)
{
    int ret;
    struct sockaddr_in userend = tmp->userend;
    socklen_t len = tmp->len;

    chat_t *send;
    send = (chat_t *)malloc(size);
    if (send == NULL) {
        perror("send");
        exit(1);
    }

    memset(send, 0, size);

    strncpy(send->information.buf, buf, strlen(buf));
    send->packetlen = size;

    ret = sendto(sd, send, size, 0, (struct sockaddr *)&userend, len);
    if (ret == -1) {
        printf("sendto");
        exit(1);
    }

    free(send);
    return;


}

void listmusic_t(char *str)
{
    int fd, ret, res;
    char tmp[BUFSISE];
    fd = open("music/musiclist", O_RDONLY);
    if (fd == -1) {
        perror("open");
        exit(1);
    }

    while(1) {

        memset(tmp, 0, BUFSISE);
        ret = read(fd, tmp, BUFSISE);
        if (ret == -1) {
            perror("read");
            exit(1);
        }

        if (ret == 0) {
            break;
        }

        strncpy(str, tmp, ret);
        str = str + ret;
    }

    return;

}

static void get_song_name(char *information, char *songname)
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



static void order_list_excut(int sd, chat_t *buf, chat_t *head)
{
    int size;
    chat_t *tmp;

    char str[1024];
    char songname[BUFSISE];
    char song_path[BUFSISE];
    memset(str, 0, 1024);
    if (strcmp(buf->information.buf, "list -all") == 0) {
        display_list_all_excut(str, head);  

        tmp = list_find(head, buf->name.buf);
        if (tmp == NULL) {
            printf("nobody\n");
            fflush(stdout);
        }
        size = sizeof(chat_t) + strlen(str);
        send_to_client(sd, tmp, str, size);
    }

    else if (strcmp(buf->information.buf, "listenmusic") != 0 && strncmp(buf->information.buf, "listenmusic", strlen("listenmusic")) == 0) {

        get_song_name(buf->information.buf, songname);
        sprintf(song_path, "music/%s", songname);
        if (access(song_path, F_OK)) {
            return;
            sprintf(str, "song name is wrong");
            tmp = list_find(head, buf->name.buf);
            if (tmp == NULL) {
                printf("nobody\n");
                fflush(stdout);
            }
 //           size = sizeof(chat_t) + strlen(str);
   //         send_to_client(sd, tmp, str, size);
        }

        else {
            pool_t * pool_buf;
            pool_buf = (pool_t *)malloc(sizeof(pool_t));
            if (pool_buf == NULL) {
                perror("malloc");
                exit(1);
            }

            memset(pool_buf, 0, sizeof(pool_t));
            strncpy(pool_buf->path, song_path, strlen(song_path));
            strncpy(pool_buf->ipaddr, buf->clientipaddr, strlen(buf->clientipaddr));
            pool_buf->port = buf->clientport;

            pool_work(pool_buf);
            return;

        }

    }


    else if (strcmp(buf->information.buf, "listmusic") == 0) {

        listmusic_t(str);

        tmp = list_find(head, buf->name.buf);
        if (tmp == NULL) {
            printf("nobody\n");
            fflush(stdout);
        }
        size = sizeof(chat_t) + strlen(str);
        send_to_client(sd, tmp, str, size);
    }

    else if (strcmp(buf->information.buf, "list -hidden") == 0) {
        display_list_hidden_excut(str, head);  

        tmp = list_find(head, buf->name.buf);
        if (tmp == NULL) {
            printf("nobody\n");
            fflush(stdout);
        }
        size = sizeof(chat_t) + strlen(str);
        send_to_client(sd, tmp, str, size);
    }

    else if (strcmp(buf->information.buf, "list") == 0) {
        display_list_excut(str, head);  

        tmp = list_find(head, buf->name.buf);
        if (tmp == NULL) {
            printf("nobody\n");
            fflush(stdout);
        }
        size = sizeof(chat_t) + strlen(str);
        send_to_client(sd, tmp, str, size);
    }



}

void get_name_information(chat_t *buf, char *name, char *information)
{
    char *p, *str, *q;
    q = name;
    p = buf->information.buf;
    str = buf->information.buf;

    if (strncmp(p, "chat", strlen("chat")) == 0) {
        p = p + 4;
    }

    else if (strncmp(p, "sendfile", strlen("sendfile")) == 0) {
        p = p + 8;
    }

    else if (strncmp(p, "nofile", strlen("nofile")) == 0) {
        p = p + 6;
    }

    else if (strncmp(p, "rxfile", strlen("rxfile")) == 0) {
        p = p + 6;
    }

    else if (strncmp(p, "overtime", strlen("overtime")) == 0) {
        p = p + 24;
    }
    while(1) {
        if (*p != ' ' || *p == '\0') {
            break;
        }
        p++;
    }

    while(1) {
        if (*p == ' ' || *p == '\0') {
            *name = '\0';
            break;
        }
        *name++ = *p++;
    }

    while(1) {
        if (*p != ' ' || *p == '\0') {
            break;
        }
        p++;
    }

    if (strncmp(str, "chat", strlen("chat")) == 0) {
        sprintf(information, "%s from %s", p, buf->name.buf);
        return;
    }

    else if (strncmp(str, "sendfile", strlen("sendfile")) == 0) {
        sprintf(information, "sendfile %s from %s", p,  buf->name.buf);
        return;
    }

    else if (strncmp(str, "nofile", strlen("nofile")) == 0) {
        sprintf(information, "nofile %s from %s", p,  buf->name.buf);
        return;
    }

    else if (strncmp(str, "rxfile", strlen("rxfile")) == 0) {
        sprintf(information, "rxfile %s %s from %s", q, p,  buf->name.buf);
        return;
    }
    else if (strncmp(str, "overtime", strlen("overtime")) == 0) {
        sprintf(information, "overtime cancle send %s from %s", p,  buf->name.buf);
        return;
    }
}


void click_status_set(chat_t* head, chat_t* buf)
{
    chat_t *tmp;
    tmp = list_find(head, buf->name.buf);
    if (tmp == NULL) {
        printf("tmp = NULL\n");
        return;
    }
    tmp->click = ON;
    return;
}


void server_work(int sd, chat_t *head, chat_t *buf)
{
    chat_t *tmp;
    char *p, *q;
    int size;
    q = buf->information.buf;
    if (buf->click == ON) {
        click_status_set(head, buf);
        return;
    }


    else if (order_list(buf) == 0) {
        order_list_excut(sd, buf, head);

    }
    else if (strncmp(buf->information.buf, "exit", strlen("exit")) == 0) {
        tmp = list_find(head, buf->name.buf);
        tmp->status = UNLINE;
        return;

    }


    else if (strncmp(buf->information.buf, "set status online", strlen("set status online")) == 0) {
        tmp = list_find(head, buf->name.buf);
        tmp->status = ONLINE;
        return;

    }

    else if (strncmp(buf->information.buf, "set status getoff", strlen("set status getoff")) == 0) {
        tmp = list_find(head, buf->name.buf);
        tmp->status = GETOFF;
        return;
    }   

    else if (strncmp(buf->information.buf, "set status hidden", strlen("set status hidden")) == 0) {
        tmp = list_find(head, buf->name.buf);
        tmp->status = HIDDEN;
        return;

    }

    else if (strncmp(buf->information.buf, "set nickname", strlen("set nickname")) == 0) {
        tmp = list_find(head, buf->name.buf);
        p = buf->information.buf + 12;
        while(1) {
            if (*p != ' ') {
                break;
            }

            p++;
        }

        strncpy(tmp->nickName.buf, p, strlen(p));
        return;
    }

    if (strncmp(q, "chat", strlen("chat")) == 0 || strncmp(q, "sendfile", strlen("sendfile")) == 0 || strncmp(q, "nofile", strlen("nofile")) == 0 || strncmp(q, "rxfile", strlen("rxfile")) == 0 || strncmp(q, "overtime", strlen("overtime")) == 0) {
        char name[BUFSISE];
        char information[BUFSISE];
        memset(name, 0, BUFSISE);
        memset(information, 0, BUFSISE);
        get_name_information(buf, name, information);

        tmp = list_find(head, name);
        if (tmp == NULL) {
            return;
        }
        if (tmp->status == UNLINE && strncmp(buf->information.buf, "chat", strlen("chat")) == 0) {
            getoff_information *node;
            getoff_information *p;
            int size;
            size = sizeof(getoff_information) + strlen(information);
            node = (getoff_information *)malloc(size);
            if (node == NULL) {
                perror("malloc");
                exit(1);
            }

            memset(node, 0, size);
            strncpy(node->buf, information, strlen(information));
            node->next = NULL;
            if (tmp->information.argv != NULL) {
                fflush(stdout);
                for (p = tmp->information.argv; p->next != NULL; p = p->next) {
                }
                p->next = node;
            }

            else {
                tmp->information.argv = node;
            }
            return;

        }

        else if ( tmp->status != UNLINE) {
            size = sizeof(chat_t) + strlen(information);
            send_to_client(sd, tmp, information, size);
            return;
        }

    }

    return;   
}
