#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>
#include <signal.h>

#define SOCK_MyTCP SOCK_STREAM
#define MAX_SIZE 50

// Jai Shree Ram
typedef struct my_n
{
    char str[5000];
    int sz;
    struct my_n* next;    
} node;

typedef struct my_tab
{
    node *head;
    node *tail;
    int sz;
    int sockfd;

}my_table;

int my_socket(int domain, int type, int protocol);
int my_bind(int sockfd, const struct sockaddr *addr,socklen_t addrlen);
int my_listen(int sockfd, int backlog);
int my_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int my_connect(int sockfd, const struct sockaddr *addr,socklen_t addrlen);
ssize_t my_send(int sockfd, const char *buf, size_t len, int flags);
ssize_t my_recv(int sockfd, char *buf, size_t len, int flags);
int my_close(int fd);
void my_push(my_table* s,node *a);
node* my_del(my_table* s);




