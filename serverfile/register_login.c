#include "chat.h"

int read_line(int fd, char * const buf)
{
    char tmp[1];
    int ret, i = 0;
    while(1) {
        ret = read(fd, tmp, 1);
        if (ret == -1) {
            perror("read");
            exit(1);
        }

        if (ret == 0) {

            return 0;
        }

        if (tmp[0] == '\n') {
            buf[i] = '\0';
            return 1;
        }

        buf[i] = tmp[0];
        i++;
    }
}

int read_name_t(char *buf, char *name)
{
    int i = 0; 
    while(1) {
        name[i] = buf[i]; 
        if (buf[i] == '#') {
            name[i] = '\0';
            break;
        }

        i++;
    }
}

int name_ifexist(int fd, const chat_t *buf)
{
    int ret, res, i;
    char tmp[BUFSISE];
    char name[BUFSISE];
    while(1) {
        ret = read_line(fd, tmp);
        if (ret == 0) {
            return 0;
        }

        read_name_t(tmp, name);

        if (ret == 1) {
            res = strcmp(name,  buf->name.buf);
            if (res == 0) {
                return -2;
            }
        }

    }
}

void writeintofile(const chat_t *buf, int fd)
{
    int ret;

    char tmp[BUFSISE];
    char passwdbuf[BUFSISE];
    strcpy(passwdbuf, (char *)crypt(buf->passwd.buf, JAR));

    sprintf(tmp, "%s#%s#\n", buf->name.buf, passwdbuf);
    ret = write(fd, tmp, strlen(tmp));
    if (ret == -1) {
        perror("write");
        exit(1);
    }

    return;


}
int chatregister(const chat_t *buf) 
{
    if (buf == NULL) {
        fprintf(stderr, "chat_buf error");
        exit(1);
    }

    int fd, ret;


    fd = open("data/contracts.db", O_CREAT | O_RDWR);
    if (fd == -1) {
        perror("open");
        exit(1);
    }

    ret = name_ifexist(fd, buf);
    if (ret == -2) {
        close(fd);
        return -2;
    }

    if (ret == 0) {
        writeintofile(buf, fd);
        close(fd);
        return 2;
    }
}


int login_check(const char *buf, int fd) 
{
    char tmp[BUFSISE];
    char name[BUFSISE];
    int ret, res;
    while(1) {
        ret = read_line(fd, tmp);
        if (ret == 0) {
            return -1;
        }


        if (ret == 1) {
            res = strcmp(tmp, buf);
            if (res == 0) {
                return 2;
            }

        }
    }
}                                                                       

int login_t(const chat_t *buf) 
{
    int fd, ret;
    char tmp[BUFSISE];

    fd = open("data/contracts.db", O_RDONLY);
    if (fd == -1) {
        perror("open");
        exit(1);
    }

    sprintf(tmp, "%s#%s#", buf->name.buf, (char *)crypt(buf->passwd.buf, JAR));
    ret = login_check(tmp, fd);
    if (ret == -1) {
        close(fd);
        return -1;
    }

    if (ret == 2) {
        close(fd);
        return 1;
    }

}


