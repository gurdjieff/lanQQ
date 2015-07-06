#include "chat.h"

chat_t *list_head_init(void)
{
    chat_t *head = NULL;
    head = (chat_t *)malloc(sizeof(chat_t));
    if (head == NULL) {
        perror("malloc");
        exit(1);
    }

    memset(head, 0, sizeof(chat_t));
    head->next = NULL;
    return head;

}

void inser_node(chat_t* head, chat_t *node)
{
    chat_t *tmp;
    for (tmp = head; tmp->next != NULL; tmp = tmp->next);
    tmp->next = node;
}

chat_t * list_find(chat_t * head, char *name)
{
    chat_t *tmp;
    for(tmp = head->next; tmp != NULL; tmp = tmp->next)
    {
        if (strcmp(tmp->name.buf, name) == 0) {
            return tmp;
        }
    }
    return NULL;
}

list_distroy(chat_t * head)
{
    chat_t *tmp;
    for (tmp = head->next; tmp != NULL; tmp = tmp->next)
    {
        free(head);
        head = tmp;
    }

}


int read_one_line(int fd, char *buf)
{
    int ret, i = 0;
    char tmp[1];
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
            buf[i] = '\0';;
            return 1;
        }

        buf[i] = tmp[0];
        i++;

    }
}
void display(chat_t *head)
{
    chat_t *tmp;
    for (tmp = head->next; tmp != NULL; tmp = tmp->next) {
        printf("name: %s\n", tmp->name.buf);
        printf("nikename: %s\n", tmp->nickName.buf);
        printf("status: %d\n", tmp->status);

    }
    return;
    
}

void read_name(char *buf, char *name)
{
    int i = 0;
    while(1) {
        if (buf[i] == '#') {
            name[i] = '\0';
            break;
        }
        name[i] = buf[i];
        i++;
        
    }

    return;
}

void node_creat_insert(chat_t *head, char *name)
{
    chat_t *node;
    node = (chat_t *)malloc(sizeof(chat_t) + strlen(name)); 
    if (node == NULL) {
        perror("node");
        exit(1);
    }

    memset(node, 0, sizeof(chat_t) + strlen(name));
    node->next = NULL;
    strncpy(node->name.buf, name, strlen(name));
    strncpy(node->nickName.buf, "none", strlen("none"));
    node->status = UNLINE;
    node->click = OFF;
    node->information.argv = NULL;
    
    inser_node(head, node);

}

chat_t * list_creat()
{
    int ret, fd;
    char buf[BUFSISE];
    char name[BUFSISE];
    chat_t *head = NULL;
    head = list_head_init();
    fd = open("data/contracts.db", O_RDONLY);
    if (fd == -1) {
        perror("open");
        exit(1);
    }
    while(1) {
        ret = read_one_line(fd, buf);
        if (ret == 0) {
            
            break;
        }

        read_name(buf, name);
        node_creat_insert(head, name);
    }

    close(fd);
    return head;
}
