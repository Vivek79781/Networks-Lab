#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>

void write_log(char *client_ip,char *method,char *url,int client_port){
    FILE *fp = fopen("AccessLog.txt","a");
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char date[100];
    char time[100];
    strftime(date,sizeof(date),"%d/%m/%y",tm);
    strftime(time,sizeof(time),"%H/%M/%S",tm);
    fprintf(fp,"%s:%s:%s:%d:%s:%s\n" ,date,time,client_ip,client_port,method,url);
    fclose(fp);
    return;
}

void handle_get_request(int cli_sockfd, char *url, char **header_fields, int header_field_count){
    if(access(url,F_OK) == -1){
        send(cli_sockfd,"HTTP/1.1 404 Not Found\r\n\0",25,0);
    }
    else{
        int fd = open(url,O_RDONLY);
        if(fd == -1){
            send(cli_sockfd,"HTTP/1.1 403 Forbidden Request\r\n\0",33,0);
        }
        char content_type[100];
        int i = 0;
        while(i < header_field_count && strncmp(header_fields[i],"Accept:",7) != 0)
            i++;
        if(i == header_field_count){
            strcpy(content_type,"text/html");
        } else {
            char *token = strtok(header_fields[i],":");
            token = strtok(NULL,":");
            // strcpy(content_type,token);
            while(token[0] == ' ')
                token++;
            strcpy(content_type,token);
        }
        char content_language[100];
        strcpy(content_language,"en-us");
        char cache_control[100];
        strcpy(cache_control,"no-store");
        time_t t = time(NULL);
        t += 3*24*60*60;
        char expires[100];
        strcpy(expires,ctime(&t));
        expires[strlen(expires) - 1] = '\r';
        struct stat file_stat;
        stat(url,&file_stat);
        char last_modified[100];
        strcpy(last_modified,ctime(&file_stat.st_mtime));
        last_modified[strlen(last_modified) - 1] = '\r';
        int content_length = 0;
        char ch;
        char *content = (char *)malloc(sizeof(char));
        content[0] = '\0';
        while(read(fd,&ch,1) != 0){
            content_length++;
            content = (char *)realloc(content,(content_length + 1)*sizeof(char));
            content[content_length - 1] = ch;
            content[content_length] = '\0';
        }
        // content_length;
        close(fd);
        char response[10000] = {0};
        sprintf(response,"HTTP/1.1 200 OK\r\nExpires: %s\nCache-Control: %s\r\nContent-Language: %s\r\nContent-Length: %d\r\nContent-Type: %s\r\nLast-Modified: %s\n\r\n",
                        expires,
                        cache_control,
                        content_language,
                        content_length,
                        content_type,
                        last_modified
                        );
        printf("%s",response);
        char *actual_response = (char *)malloc((strlen(response) + content_length)*sizeof(char));
        strcpy(actual_response,response);
        for(int i = 0; i < content_length; i++)
            actual_response[strlen(response) + i] = content[i];
        int response_len = strlen(response) + content_length;        
        // printf("%d\n",response_len);
        send(cli_sockfd,actual_response,response_len + 1,0);
        // printf("Sent\n");
        free(actual_response);
        free(content);
    }
    
}

void handle_put_request(int cli_sockfd, char *url, char **header_fields, int header_field_count, char* content, int recv_len){
    int fd = open(url,O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if(fd == -1){
        send(cli_sockfd,"HTTP/1.1 403 Forbidden Request\n\n\0",33,0);
    }
    else{
        int content_length = 0;
        int i = 0;
        while(i < header_field_count && strncmp(header_fields[i],"Content-Length:",15) != 0)
            i++;
        if(i == header_field_count){
            send(cli_sockfd,"HTTP/1.1 400 Bad Request\n\n\0",27,0);
            return;
        } else {
            char *token = strtok(header_fields[i],":");
            token = strtok(NULL,":");
            while(token[0] == ' ')
                token++;
            content_length = atoi(token);
        }
        i = 0;
        int count = 0;
        while(i < recv_len){
            write(fd,&content[i],1);
            if(content[i] == '\0')
                count++;
            i++;
        }
        // printf("%d\n",count);
        char *extra = (char *)malloc((content_length - recv_len + 1)*sizeof(char));
        int total = i;
        int extra_count = 0;
        while(total < content_length){
            char *temp = (char *)malloc(100*sizeof(char));
            int n = recv(cli_sockfd,temp,100,0);
            if(n == -1){
                printf("Receive failed!\n");
                exit(1);
            }
            if(n == 0)
                break;
            for(int i = 0; i < n; i++)
                extra[extra_count + i] = temp[i];
            total += n;
            extra_count += n;
        }
        for(int i = 0; i < extra_count-1; i++)
            write(fd,&extra[i],1);
        // send(cli_sockfd,"HTTP/1.1 200 OK\n\n\0",18,0);
        char response[10000] = {0};
        // last modified
        struct stat file_stat;
        stat(url,&file_stat);
        char last_modified[100];
        strcpy(last_modified,ctime(&file_stat.st_mtime));
        last_modified[strlen(last_modified) - 1] = '\r';
        // content type
        char content_type[100];
        i = 0;
        while(i < header_field_count && strncmp(header_fields[i],"Content-Type:",13) != 0)
            i++;
        if(i == header_field_count){
            send(cli_sockfd,"HTTP/1.1 400 Bad Request\n\n\0",27,0);
            return;
        } else {
            char *token = strtok(header_fields[i],":");
            token = strtok(NULL,":");
            while(token[0] == ' ')
                token++;
            strcpy(content_type,token);
        }
        sprintf(response,"HTTP/1.1 200 OK\r\nContent-Language: %s\r\nContent-Length: %d\r\nContent-Type: %s\r\nLast-Modified: %s\n",
                        "en-us",
                        content_length,
                        content_type,
                        last_modified
                        );
        printf("%s",response);
        send(cli_sockfd,response,strlen(response)+1,0);
        free(extra);

    }
}

void handle_request(int cli_sockfd,struct sockaddr_in cli_addr){
    // printf("Forked123\n");
    int n = 0;
    char *buf = NULL;
    // buf[0] = '\0';
    int total = 0;
    while(1){
        char *temp = (char *)malloc(100*sizeof(char));
        n = recv(cli_sockfd,temp,100,0);
        if(n == -1){
            printf("Receive failed!\n");
            exit(1);
        }
        buf = (char *)realloc(buf,(total + n )*sizeof(char));
        for(int i = 0; i < n; i++)
            buf[total + i] = temp[i];
        total += n;
        if(temp[n-1] == '\0'){
            free(temp);
            break;
        }
        free(temp);
    }
    if(n == -1){
        printf("Receive failed!\n");
        exit(1);
    }
    char *method = (char *)malloc(100*sizeof(char));
    char *url = (char *)malloc(100*sizeof(char));
    char **header_fields = (char **)malloc(100*sizeof(char *));
    for(int i = 0;i < 100;i++)
        header_fields[i] = (char *)malloc(100*sizeof(char));
    int header_field_count = 0;
    int i = 0,j = 0;
    while(buf[i] != ' '){
        method[j++] = buf[i++];
    }
    method[j] = '\0';
    i++;
    j = 0;
    while(buf[i] != ' '){
        url[j++] = buf[i++];
    }
    url[j] = '\0';
    i++;
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET,&cli_addr.sin_addr,client_ip,INET_ADDRSTRLEN);
    int client_port = ntohs(cli_addr.sin_port);
    write_log(client_ip,method,url,client_port);
    printf("Request Received\n%s %s",method,url);
    header_field_count = 0;
    while(buf[i] != '\r'){
        printf("%c",buf[i]);
        i++;
    }
    printf("\r\n");
    i+=2;
    while(i < strlen(buf) && buf[i] != '\r'){
        j = 0;
        while(i < strlen(buf) && buf[i] != '\r'){
            header_fields[header_field_count][j++] = buf[i++];
        }
        // printf("%s\n",method);
        header_fields[header_field_count][j] = '\0';
        header_field_count++;
        i+=2;
    }
    i+=2;
    // printf("%d\n",buf[i]);
    for(int k = 0;k < header_field_count;k++)
        printf("%s\r\n",header_fields[k]);
    // printf("%d %d\n",total,i);
    printf("\nResponse Sent\n");
    if(strcmp(method,"GET") == 0){
        handle_get_request(cli_sockfd,url,header_fields,header_field_count);
    } else if(strcmp(method,"PUT") == 0){
        handle_put_request(cli_sockfd,url,header_fields,header_field_count,buf + i,total - i);
    } else {
        char *response = "HTTP/1.1 400 Bad Request\r\n";
        printf("%s",response);
        send(cli_sockfd,response,strlen(response),0);
    }
    free(buf);
    free(method);
    free(url);
    free(header_fields);
    
    close(cli_sockfd);
    //free(header_field_count);
    // printf("Closed\n");

}

int main(){
    int serv_sockfd,cli_sockfd;
    struct sockaddr_in serv_addr,cli_addr;
    int cli_len = sizeof(cli_addr);
    serv_sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(serv_sockfd == -1){
        printf("Socket create failed!\n");
        exit(1);
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(serv_sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) == -1){
        printf("Bind failed!\n");
        exit(1);
    }
    if(listen(serv_sockfd,5) == -1){
        printf("Listen failed!\n");
        exit(1);
    }
    while(1){
        cli_sockfd = accept(serv_sockfd,(struct sockaddr *)&cli_addr,&cli_len);
        if(cli_sockfd == -1){
            printf("Accept failed!\n");
            exit(1);
        }
        if(fork() == 0){
            close(serv_sockfd);
            // printf("Forked\n");
            handle_request(cli_sockfd,cli_addr);
            close(cli_sockfd);
            exit(0);
        } 
        // else {
        // }
        close(cli_sockfd);
    }
    close(serv_sockfd);
}
