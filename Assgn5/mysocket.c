#include "mysocket.h"

my_table* R;
my_table* S;

// threads functions

pthread_mutex_t R_lock;
pthread_mutex_t S_lock;
pthread_t send_th,recv_th;
int cli = 0;
void *send_thread(void *arg){
    while(1){
        while(S->sz==0)
            sleep(1);
        pthread_mutex_lock(&S_lock);
        node *p=my_del(S);
        pthread_mutex_unlock(&S_lock);
        char buff[5010];
        char len[10];
        sprintf(len,"%d\r\n",p->sz);
        for(int i = 0;i < strlen(len);i++)
            buff[i] = len[i];
        for(int i = 0;i < p->sz;i++)
            buff[i + strlen(len)] = p->str[i];
        int n=0;
        n=send(S->sockfd,buff,strlen(len)+p->sz,0);

        if(n<0)
        {
            perror("It is killing here\n");
        }
        free(p);
        sleep(1);
    }
}

void *recv_thread(void *arg){
    while(1){
        while(R->sz >= MAX_SIZE || cli == 0)
            sleep(1);
        node *a = (node *)malloc(sizeof(node));
        
        int len=0,i=0,n=0;
        char buf[5010];            
            n = recv(R->sockfd,buf,5010,0);
            while(1){
                int flag=0;
                for(;i<n-1;i++){
                    if(buf[i]=='\r' && buf[i+1]=='\n'){
                        flag=1;
                        break;
                    }
                }
                if(flag)
                    {
                        char arr[i+1];
                        for(int j=0;j<i;j++)
                            arr[j]=buf[j];
                        arr[i]='\0';
                        len=atoi(arr);
                        break;
                    }
                int nn=recv(R->sockfd,buf+n,5010-n,0);
                n+=nn;
            }

            // recv till len
            while(n<len + i + 2){
                int nn=recv(R->sockfd,buf+n,5010-n,0);
                n+=nn;
            }

            a->sz = len;    
        for(int j=i+2;j<n;j++)
            a->str[j-i-2]=buf[j];

        a->next=NULL;
        pthread_mutex_lock(&R_lock);
        my_push(R,a);
        pthread_mutex_unlock(&R_lock);
    }
}

void my_push(my_table* s,node *a)
{
    if(s->sz==0)
    {
        s->head=a;
        s->tail=a;
        s->sz=1;
    }
    else
    {
        s->tail->next=a;
        s->tail=a;
        s->sz+=1;
    }
}

node *my_del(my_table* s)
{
    if(s->sz==1)
    {
        node *p=s->head;
        s->head=NULL;
        s->tail=NULL;
        s->sz=0;
        return p;
    }
    else
    {
        node *p=s->head;
        s->head=s->head->next;
        s->sz-=1;
        return p;
    }
}



int my_socket(int domain, int type, int protocol)
{
    pthread_mutex_init(&R_lock,NULL);
    pthread_mutex_init(&S_lock,NULL);

    int sockfd;
    R=(my_table *)malloc(sizeof(my_table));
    S=(my_table *)malloc(sizeof(my_table));
    R->head=NULL;
    R->tail=NULL;
    S->head=NULL;
    R->tail=NULL;

    R->sz=0;
    S->sz=0;

    R->sockfd=S->sockfd=sockfd=socket(AF_INET,SOCK_STREAM,0);

    // threads create and join
    
    pthread_create(&send_th,NULL,send_thread,NULL);
    pthread_create(&recv_th,NULL,recv_thread,NULL);
    
    return sockfd;
}

int my_bind(int sockfd, const struct sockaddr *addr,socklen_t addrlen)
{
    return bind(sockfd,addr,addrlen);
}

int my_listen(int sockfd, int backlog)
{
    return listen(sockfd,backlog);
}

int my_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    int newsockfd;
    newsockfd=accept(sockfd,addr,addrlen);
    pthread_mutex_lock(&S_lock);
    S->sockfd=newsockfd;
    pthread_mutex_unlock(&S_lock);
    pthread_mutex_lock(&R_lock);
    R->sockfd=newsockfd;
    pthread_mutex_unlock(&R_lock);
    cli = 1;
    return newsockfd;
}

int my_connect(int sockfd, const struct sockaddr *addr,socklen_t addrlen)
{
    cli = 1;
    return connect(sockfd,addr,addrlen);
}

ssize_t my_send(int sockfd, const char *buf, size_t len, int flags)
{
    while(S->sz >= MAX_SIZE)sleep(1);
    node *a = (node *)malloc(sizeof(node));
    a->sz=len;
    for(int i=0;i<len;i++)
    {
        a->str[i] = buf[i];
    }
    a->next=NULL;
    pthread_mutex_lock(&S_lock);
    my_push(S,a);
    pthread_mutex_unlock(&S_lock);

    return len;

}

ssize_t my_recv(int sockfd, char *buf, size_t len, int flags)
{
    while(R->sz==0)
    {
        sleep(1);
    }
    pthread_mutex_lock(&R_lock);
    node *p=my_del(R);
    pthread_mutex_unlock(&R_lock);
    int len1 = (p->sz<len)?p->sz:len;
    for(int i=0;i<len1;i++)
    {
        buf[i]=p->str[i];
    }
    free(p);
    return len1;
}

int my_close(int fd)
{
    int sockfd;
    sleep(5);
    sockfd=R->sockfd;
    // thread destroy
    if(sockfd==fd)
    {
        pthread_mutex_lock(&R_lock);
        free(R);
        pthread_mutex_unlock(&R_lock);

        pthread_mutex_lock(&S_lock);
        free(S);
        pthread_mutex_unlock(&S_lock);
        // kill pthread threads
        pthread_cancel(send_th);
        pthread_cancel(recv_th);
    }

    return close(fd);
}




