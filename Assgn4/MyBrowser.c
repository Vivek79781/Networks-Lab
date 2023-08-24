#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <dirent.h>
#include <errno.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <poll.h>



int main()
{
    while(1)
    {
        printf("MyOwnBrowser> ");
        char *str=NULL;
        int sz1=0;
        //printf("Enter the expression\n");
        int len1=getline(&str,&sz1,stdin);
        if(str[0]=='Q' && str[1]=='U' && str[2]=='I' && str[3]=='T')
        break;

        if(str[0]=='G' && str[1]=='E' && str[2]=='T')
        {
            int port=80;
            int i=0;
            while(!((str[i]>='0' && str[i]<='9') || str[i]=='.') )i++;
            //printf("%d\n",st);
            int st=i;
            while((str[i]>='0' && str[i]<='9') || str[i]=='.')i++;
            int en=i-1;

            char *ip=(char *)malloc((en-st+2)*sizeof(char));

            for(int j=st;j<=en;j++)
            ip[j-st]=str[j];
            ip[en-st+1]='\0';

            //printf("IP is : %s\n",ip);

            i=0;
            int cn=0,pos=0;
            while(i<strlen(str))
            {
                if(str[i]==':')
                {
                    cn++;;pos=i;
                }
                i++;
            }
            if(cn==2)
            {
                int l=strlen(str);
                pos++;
                char *prt=(char *)malloc((l-pos+1)*sizeof(char));

                for(int j=pos;j<l;j++)
                prt[j-pos]=str[j];

                prt[l-pos]='\0';

                port=atoi(prt);
                pos--;
                free(prt);
            }

            //printf("Port is %d\n",port);

            int sockfd;
            struct sockaddr_in serv_addr;

            if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                perror("Unable to create socket\n");
                exit(0);
            }
            //printf("Socket Created\n");

            serv_addr.sin_family	= AF_INET;
            inet_aton(ip, &serv_addr.sin_addr);
            //printf("%d\n",strcmp(ip,"10.147.234.153"));
            serv_addr.sin_port	= htons(port);

            
            if ((connect(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr))) < 0) 
            {
                printf("Unable to access server\n");
                free(str);
                continue;
            }

            //printf("Connection DOne\n");

            char *sans = (char *)malloc(10240000*sizeof(char));

            strcpy(sans,"GET ");

            if(pos<en)
            {
                int l=strlen(str);
                l--;// back slash n ke liye
                char * url=(char *)malloc((l-en)*sizeof(char));

                for(int j=en+1;j<l;j++)
                {
                    url[j-en-1]=str[j];
                }
                url[l-en-1]='\0';
                //printf("%s\n",url);
                strcat(sans,url);
                free(url);
            }
            else
            {
                int l=pos;
                //l--;
                char * url=(char *)malloc((l-en)*sizeof(char));

                for(int j=en+1;j<l;j++)
                {
                    url[j-en-1]=str[j];
                }
                url[l-en-1]='\0';
                //printf("%s\n",url);
                strcat(sans,url);
                free(url);
            }

            strcat(sans," HTTP/1.1\r\n");


            strcat(sans,"Host: ");
            strcat(sans,ip);
            strcat(sans,"\r\n");
            strcat(sans,"Connection: close\r\n");
            // time_t t;   // not a primitive datatype
            // time(&t);
            //strcat(sans,ctime(&t));

            char* ext;

            if(pos<en)
            {
                int l=strlen(str);

                //l--;
                int j=l-1;
                while(str[j]!='.')
                {
                    j--;
                }

                //j++;
                ext=(char *)malloc((l-j+1)*sizeof(char));

                for(int k=j;k<l;k++)
                {
                    ext[k-j]=str[k];
                }
                ext[l-j]='\0';

            }
            else
            {
                int l=pos;

                //l--;
                int j=l-1;
                while(str[j]!='.')
                {
                    j--;
                }

                //j++;
                ext=(char *)malloc((l-j+1)*sizeof(char));

                for(int k=j;k<l;k++)
                {
                    ext[k-j]=str[k];
                }
                ext[l-j]='\0';
            }

            if(strcmp(ext,".html")==0)
            {
                strcat(sans,"Accept: text/html\r\n");
            }
            else if(strcmp(ext,".pdf")==0)
            {
                strcat(sans,"Accept: application/pdf\r\n");
            }
            else if(strcmp(ext,".jpg")==0)
            {
                strcat(sans,"Accept: image/jpeg\r\n");
            }
            else
            {
                strcat(sans,"Accept: text/*\r\n");
            }

            strcat(sans,"Accept-Language: en-us,en;q=0.8\r\n");

            time_t current_time;
            struct tm *time_info;
            char buffer[80];

            time(&current_time);
            time_info = gmtime(&current_time);
            time_info->tm_mday -= 2;
            mktime(time_info);

            strftime(buffer, 80, "Date: %a, %d %b %Y %T GMT\r\n", time_info);
            strcat(sans,buffer);
            time_info->tm_mday += 2;
            char bufut[80];
            strftime(bufut, 80, "Date: %a, %d %b %Y %T GMT\r\n", time_info);
            strcat(sans,bufut);
            strcat(sans,"\r\n");
            printf("Request sent\n");
            printf("%s",sans);
            send(sockfd,sans,strlen(sans)+1,0);
            // memset(sans,0,sizeof(sans));
            /*
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
            
            */
            
            struct pollfd fdset;
                fdset.fd=sockfd;
                fdset.events=POLLIN;

                int ret=poll(&fdset,1,3000);
                if(ret<=0)
                {
                    printf("Timeout server\n");
                    close(sockfd);
                    free(str);
                    continue;
                }
            char *response = NULL;
            int recv_len = 1;//=recv(sockfd,response,10240000,0);
            int total_len=0;
            while(1)
            {
                char *temp = (char *)malloc(100*sizeof(char));
                recv_len=recv(sockfd,temp,100,0);
                if(recv_len==0)
                {
                    free(temp);
                    break;
                }
                //printf("%d %d %s\n",total_len, recv_len,response);
                response = (char *)realloc(response,(total_len + recv_len + 1)*sizeof(char));
                for(int i = 0; i < recv_len; i++)
                    response[total_len + i] = temp[i];
                total_len+=recv_len;
                free(temp);
            }
            //total_len--;
            //printf("%d\n",total_len);
            //free(str);
            int stl=strlen("HTTP/1.1 ");
            int status=1;
            if(response[stl]=='2' && response[stl+1]=='0' && response[stl+2]=='0')
            {
                status=0;
            }
            else
            {
                status=1;
                if(response[stl]=='4' && response[stl+1]=='0' && response[stl+2]=='0')
                {
                    printf("BAD Request\n");
                }
                else if(response[stl]=='4' && response[stl+1]=='0' && response[stl+2]=='4')
                {
                    printf("URL NOT FOUND\n");
                }
                else if(response[stl]=='4' && response[stl+1]=='0' && response[stl+2]=='3')
                {
                    printf("Permission denied\n");
                }
                else
                {
                    printf("Unknown Error\n");
                }

            }

            
            close(sockfd);

            if(status)
            {
                free(response);
                free(str);
                continue;
            }

            //printf("%d\n",strcpy(response_temp,response);status);

            //Parse the HTTP response
            char *content_type = NULL;
            char *content = NULL;
            char *response_temp = (char *)malloc(10240000*sizeof(char));
            //strcpy(response_temp,response);
            for(int op=0;op<total_len;op++)
            response_temp[op]=response[op];
            
            char *header = strtok(response, "\n");
            while (header != NULL) 
            {
                // Extract the content type
                if (strstr(header, "Content-Type: ") == header) 
                {
                    content_type = header + strlen("Content-Type: ");
                    break;
                }
                //Check if we have reached the end of the headers
                //printf("%c\n",header[0]);
                header = strtok(NULL, "\n");
            }

            int in=3;
            int start_c=0;
            while(in<strlen(response_temp))
            {
                if(response_temp[in]=='\n' && response_temp[in-1]=='\r' && response_temp[in-2]=='\n' && response_temp[in-3]=='\r')
                {
                    start_c=in+1;
                    //printf("hello %d\n",in);
                    break;
                }
                in++;
            }
            printf("Response Recieved is\n");
            for(int outp=0;outp<=in;outp++)
            printf("%c",response_temp[outp]);

            //printf("%d %d\n",response_temp[strlen(response_temp)-1],response_temp[strlen(response_temp)-2]);

            //Determine the file extension
            //printf("%s\n",content_type);
            char *extension = NULL;
            if (content_type != NULL) 
            {
                if (strstr(content_type, "image/jpeg") == content_type) 
                {
                    extension = ".jpg";
                } 
                else if (strstr(content_type, "text/*") == content_type) 
                {
                    extension = ext;
                }
                else if (strstr(content_type, "text/html") == content_type) 
                {
                    extension = ".html";
                }
                else if (strstr(content_type, "application/pdf") == content_type) 
                {
                    extension = ".pdf";
                }

            }

            //printf("%s\n",content_type);

            //printf("%s\n",extension);
            //printf("%s\n",response);

            //Write the content to a file
            if (extension != NULL && start_c!=0) 
            {
                char filename[100];
                sprintf(filename, "file%s", extension);
                FILE *fp = fopen(filename, "wb");
                if (fp != NULL) 
                {
                    for(int op=start_c;op<total_len-1;op++)
                    fputc(response_temp[op],fp);
                    fclose(fp);

                    // Open the file with the appropriate application
                    pid_t pid = fork();
                    if (pid == 0) 
                    {
                        // Child process
                        //printf("%s\n",filename);
                        if(strcmp(extension,".jpg")==0)
                        {
                            char *args[] = {"xdg-open", filename, NULL};
                            execvp("xdg-open", args);
                            exit(0);
                        }
                        else if(strcmp(extension,".html")==0)
                        {
                            char *args[] = {"google-chrome", filename, NULL};
                            execvp("google-chrome", args);
                            exit(0);
                        }
                        else if(strcmp(extension,".pdf")==0)
                        {
                            char *args[] = {"xdg-open", filename, NULL};
                            execvp("xdg-open", args);
                            exit(0);
                        }
                        else
                        {
                            char *args[] = {"xdg-open", filename, NULL};
                            execvp("xdg-open", args);
                            exit(0);
                        }
                        
                        

                        
                    } 
                    else 
                    {
                        // Parent process
                        int status;
                        wait(pid);
                    }
                }
            }
            else
                printf("Prabhu Aapki Jay Ho\n");
            free(sans);
            free(ip);
            free(ext);
            free(response);
        }
        else if(str[0]=='P' && str[1]=='U' && str[2]=='T')
        {
            int port=80;
            int i=0;
            while(!((str[i]>='0' && str[i]<='9') || str[i]=='.') )i++;
            //printf("%d\n",st);
            int st=i;
            while((str[i]>='0' && str[i]<='9') || str[i]=='.')i++;
            int en=i-1;

            char *ip=(char *)malloc((en-st+2)*sizeof(char));

            for(int j=st;j<=en;j++)
            ip[j-st]=str[j];
            ip[en-st+1]='\0';

            //printf("IP is : %s\n",ip);

            int second_space=-1;

            for(int i=0;i<strlen(str);i++)
            {
                if(str[i]==' ' && second_space==-1)
                {
                    second_space=i;
                }
                else if(str[i]==' ')
                {
                    second_space=i;
                }
            }

            i=0;
            int cn=0,pos=0;
            while(i<strlen(str))
            {
                if(str[i]==':')
                {
                    cn++;;pos=i;
                }
                i++;
            }
            if(cn==2)
            {
                int l=second_space;
                pos++;
                char *prt=(char *)malloc((l-pos+1)*sizeof(char));

                for(int j=pos;j<l;j++)
                prt[j-pos]=str[j];

                prt[l-pos]='\0';

                port=atoi(prt);
                pos--;
                free(prt);
            }

            //printf("Port is %d\n",port);

            int len=strlen(str);//second_space;
            len--;
                second_space++;
                char *fname=(char *)malloc((len-second_space+1)*sizeof(char));

                for(int j=second_space;j<len;j++)
                fname[j-second_space]=str[j];

                fname[len-second_space]='\0';

            //printf("%s\n",fname);

            char response[1001];
            int response_len = 0;
            int status=0;

            FILE *fp;
            if(fopen(fname,"r") == NULL)
            {
                sprintf(response,"HTTP/1.1 404 Not Found\n\n");
                response_len = strlen(response);
                printf("%s\n",response);
                status=1;
                //send(cli_sockfd,response,response_len + 1,0);
            }
            int fd = open(fname,O_RDONLY);
                if(fd == -1){
                    sprintf(response,"HTTP/1.1 403 Forbidden Request\n\n");
                    response_len = strlen(response);
                    printf("%s\n",response);
                    status=1;
                }
            if(status)
            {
                free(str);
                continue;
            }

            int sockfd;
            struct sockaddr_in serv_addr;

            if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                perror("Unable to create socket\n");
                exit(0);
            }
            //printf("Socket Created\n");

            serv_addr.sin_family	= AF_INET;
            inet_aton(ip, &serv_addr.sin_addr);
            //printf("%d\n",strcmp(ip,"10.147.234.153"));
            serv_addr.sin_port	= htons(port);

            
            //printf("%dret\n",ret);

            if ((connect(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr))) < 0) 
            {
                printf("Unable to access server\n");
                free(str);
                continue;
            }

            //printf("Connection Done\n");

            
            char *sans = (char *)malloc(10240000*sizeof(char));

            strcpy(sans,"PUT ");

            if(pos<en)
            {
                int l=second_space;
                l--;
                char * url=(char *)malloc((l-en)*sizeof(char));

                for(int j=en+1;j<l;j++)
                {
                    url[j-en-1]=str[j];
                }
                url[l-en-1]='\0';
                if(strlen(url)>1)
                strcat(sans,url);
                //printf("%s",url);
                strcat(sans,fname);
                free(url);
            }
            else
            {
                int l=pos;
                //l--;
                char * url=(char *)malloc((l-en)*sizeof(char));

                for(int j=en+1;j<l;j++)
                {
                    url[j-en-1]=str[j];
                }
                url[l-en-1]='\0';
                strcat(sans,url);
                strcat(sans,fname);
                free(url);
            }

            strcat(sans," HTTP/1.1\r\n");


            strcat(sans,"Host: ");
            strcat(sans,ip);
            strcat(sans,"\r\n");
            strcat(sans,"Connection: close\r\n");
            time_t current_time;
            struct tm *time_info;
            char buffer[80];
            time(&current_time);
            time_info = gmtime(&current_time);
            strftime(buffer, 80, "Date: %a, %d %b %Y %T GMT\r\n", time_info);
            strcat(sans,buffer);   
            strcat(sans,"Content-Language:en-us\r\n");

            int len1=strlen(str);
            len1--;

                //l--;
                int j=len1-1;
                while(str[j]!='.')
                {
                    j--;
                }

                j++;
                char * ext=(char *)malloc((len1-j+1)*sizeof(char));

                for(int k=j;k<len1;k++)
                {
                    ext[k-j]=str[k];
                }
                ext[len1-j]='\0';

            if(strcmp(ext,"html")==0)
            {
                strcat(sans,"Content-Type: text/html\r\n");
            }
            else if(strcmp(ext,"pdf")==0)
            {
                strcat(sans,"Content-Type: application/pdf\r\n");
            }
            else if(strcmp(ext,"jpg")==0)
            {
                strcat(sans,"Content-Type: image/jpeg\r\n");
            }
            else
            {
                strcat(sans,"Content-Type: text/*\r\n");
            }

            
            
            
            
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
                char chl[50];
                sprintf(chl,"Content-Length: %d\r\n\r\n",content_length);
                strcat(sans,chl);
                printf("Request Sent\n");
                printf("%s",sans);
                char *actual_response = (char *)malloc((strlen(sans) + content_length+1)*sizeof(char));
                strcpy(actual_response,sans);
                for(int i = 0; i <=content_length; i++)
                actual_response[strlen(sans) + i] = content[i];
                int total_len = strlen(sans) + content_length+1;
                //printf("%s\n",actual_response);
                if(send(sockfd,actual_response,total_len,0)==-1)printf("Jay SHree Ram\n");
                free(content);
                free(actual_response);

                free(sans);

                struct pollfd fdset;
                fdset.fd=sockfd;
                fdset.events=POLLIN;

                int ret=poll(&fdset,1,3000);
                if(ret<=0)
                {
                    printf("Timeout server\n");
                    close(sockfd);
                    free(str);
                    continue;
                }
                char * recv_ans=NULL;

                int recv_len=1;//=recv(sockfd,recv_ans,10240000,0);
                int total_length=0;
                while(recv_len>0)
                {
                    char *temp = (char *)malloc(100*sizeof(char));
                    recv_len=recv(sockfd,temp,100,0);
                    if(recv_len==0)
                    {
                        free(temp);
                        break;
                    }
                    recv_ans = (char *)realloc(recv_ans,(total_length + recv_len)*sizeof(char));
                    for(int i = 0; i < recv_len; i++)
                        recv_ans[total_length + i] = temp[i];
                    total_length+=recv_len;
                    free(temp);
                }

                int stl=strlen("HTTP/1.1 ");
            status=1;
            if(recv_ans[stl]=='2' && recv_ans[stl+1]=='0' && recv_ans[stl+2]=='0')
            {
                status=0;
            }
            else
            {
                status=1;
                if(recv_ans[stl]=='4' && recv_ans[stl+1]=='0' && recv_ans[stl+2]=='0')
                {
                    printf("BAD Request\n");
                }
                else if(recv_ans[stl]=='4' && recv_ans[stl+1]=='0' && recv_ans[stl+2]=='4')
                {
                    printf("URL NOT FOUND\n");
                }
                else if(recv_ans[stl]=='4' && recv_ans[stl+1]=='0' && recv_ans[stl+2]=='3')
                {
                    printf("Permission denied\n");
                }
                else
                {
                    printf("Unknown Error\n");
                }

            }
            if(status)continue;
                printf("Response Recieved\n%s\n",recv_ans);
                

            

            close(sockfd);
        }
        else
        {
            printf("ERROR BAD REQUEST\n");
        }
        free(str);
    }
    //free(str);
    return 0;
}