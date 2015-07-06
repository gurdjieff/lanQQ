#ifndef __CHAT_H
#define __CHAT_H

//#include "thread_pool.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <poll.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>



#define BUFSISE 512
#define IPSIZE 16
#define CLIENTIPADDR "192.168.0.1"
#define SERVERIPADDR "192.168.1.105"
#define SERVERPORT 8888
#define CLIENTPORT 8880
#define JAR "99"
#define POOLNUM 10

typedef void(* task_t)(void *);


typedef struct CLICK {
    int sd;
    struct sockaddr_in *serverend;
    socklen_t len;
    char name[1];
} click_str;


typedef enum {
    NONE,
    REGISTER,
    LOGIN,
    LOGINSUCCESS
} begin_t;

typedef enum {
    UNLINE,
    ONLINE,
    GETOFF,
    HIDDEN
} status_t;

typedef struct {
    int c;
    char buf[1];
} name_t;

typedef struct GETOFFINFORMATION{
    struct GETOFFINFORMATION *next;
    char buf[1];

} getoff_information;
typedef struct {
    int c;
    char buf[1];
} passwd_t;

typedef struct {
    int c;
    getoff_information *argv;
    char buf[1];
} information_t;


typedef struct {
    int c;
    char buf[1];
} order_t;


typedef struct {
    int c;
    char buf[BUFSISE];
} nickName_t;

typedef struct {
    int c;
    char buf[1];
} client_t;

typedef enum {
    OFF,
    ON
} click_t;

typedef struct CHAT{
    int packetlen;
    click_t click;
    begin_t login_or_register;
    order_t order;
    status_t status;
    int res;
    name_t name;
    char clientipaddr[IPSIZE];
    int clientport;
    passwd_t passwd;
    information_t information;
    struct sockaddr_in userend;
    socklen_t len;
    nickName_t nickName;
    struct CHAT *next;

} chat_t;

typedef struct POOL {
    int port;
    char ipaddr[IPSIZE];
    char path[BUFSISE];
    
} pool_t;

int pool_work(pool_t *buf);

void server_work(int sd, chat_t *head, chat_t *buf);
void pthread_create_word(chat_t *head);
void node_creat_insert(chat_t *head, char *name);
void display(chat_t *head);
chat_t * list_find(chat_t * head, char *name);
chat_t * list_creat();
int chatregister(const chat_t *buf);
int login_t(const chat_t *chat); 
int send_file_t(int sd, struct sockaddr_in *serverend, socklen_t len, char *buf_read, char *name);
int receive_file_t(int sd, struct sockaddr_in *serverend, socklen_t len, char * information, char *name);

int listen_music(int sd, struct sockaddr_in *serverend, socklen_t len, chat_t *buf);
int threadPoll_create_and_init(int num);
int threadPoll_assign_work(task_t task, void * arg, int num);


void information_save(chat_t *buf, const char *file);



#endif
