#include "chat.h"

void click_check(chat_t *head)
{
    chat_t *tmp;
    for (tmp = head->next; tmp != NULL; tmp = tmp->next) {
        if (tmp->click == ON) {
            tmp->click = OFF;
        }

        else if (tmp->click == OFF && tmp->status != UNLINE) {
            tmp->status = UNLINE;
        }
    }
    return;
}

void *pthread_handler(void *argv)
{
    pthread_detach(pthread_self());
    chat_t *head;
    head = (chat_t *)argv;
    while(1) {
        sleep(5);
        click_check(head);
            
    }

    return NULL;
    
}


void pthread_create_work(chat_t *head)
{
    pthread_t tid;
    int ret;
    ret = pthread_create(&tid, NULL, pthread_handler, head);
    if (ret) {
        fprintf(stderr, "pthread_create: %s\n", strerror(ret));
        exit(1);
    }

 //   pthread_join(tid, NULL);



    return;
}
