// 20CS10077	Vivek Jaiswal
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <dirent.h>

// Execute Command Helper Function
void execute_command(int newsockfd, char command[]){
	char buffer[1000] = {0};
	char res[1000] = {0};
	// Break the code by spaces
	char *token = strtok(command," ");
	// pwd case
	if(strcmp(command,"pwd") == 0){
		getcwd(buffer,1000);
		strcpy(res,buffer);
		strcat(res,"\n");
		// send to client in 50 sizes
		for(int i = 0;i < strlen(res)+1;i+=50){
			send(newsockfd,res+i,(strlen(res)+1-i < 50)?strlen(res)+1-i:50,0);
		}
		//send(newsockfd,res,strlen(res)+1,0);
		return;
	} else if(strcmp(token,"dir") == 0){	// dir case
		char* dir = strtok(NULL," ");
		//printf("%s\n",dir);
		// No Second Argument Case
		if(dir == NULL)
			dir = ".";
		// open dir
		DIR* d = opendir(dir);
		// error case
		if(d == NULL){
			strcpy(res,"####");
		} else {
			struct dirent* lst;
			while((lst = readdir(d)) != NULL){	// store all the names

				strcat(res, lst->d_name);
				strcat(res," ");
			}
			closedir(d);
		}
		strcat(res,"\n");
		// send to client in 50 sizes
		for(int i = 0;i < strlen(res)+1;i+=50){
			send(newsockfd,res+i,(strlen(res)+1-i < 50)?strlen(res)+1-i:50,0);
		}
		//send(newsockfd,res,strlen(res)+1,0);
		return;
	} else if(strcmp(command,"cd") == 0){	// cd case
		char* dir = strtok(NULL," ");
		if(dir == NULL){
			strcpy(res,"####\n");
		} else {
			if(chdir(dir) != 0){
				strcpy(res,"####\n");
			} else {
				getcwd(buffer,1000);
				strcpy(res,buffer);
				strcat(res,"\n");
			}
		}
		for(int i = 0;i < strlen(res)+1;i+=50){
			send(newsockfd,res+i,(strlen(res)+1-i < 50)?strlen(res)+1-i:50,0);
		}
		//send(newsockfd,res,strlen(res)+1,0);
		return;
	} else {
		strcpy(res,"$$$$\n");
		send(newsockfd,res,strlen(res)+1,0);
		return;
	}
	
	//send(newsockfd,res,strlen(res)+1,0);
}

int main() { 
    int sockfd, newsockfd; 
    struct sockaddr_in serv_addr, cli_addr; 
      
    // Create socket file descriptor 
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if ( sockfd < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
      
    memset(&serv_addr, 0, sizeof(serv_addr)); 
    memset(&cli_addr, 0, sizeof(cli_addr)); 
      
    serv_addr.sin_family    = AF_INET; 
    serv_addr.sin_addr.s_addr = INADDR_ANY; 
    serv_addr.sin_port = htons(8010); 
      
    // Bind the socket with the server address 
    if ( bind(sockfd, (const struct sockaddr *)&serv_addr,  
            sizeof(serv_addr)) < 0 ) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
    
    printf("\nServer Running....\n");
  
    listen(sockfd,5);
    while(1){
    	int clilen = sizeof(cli_addr);
    	/* The accept() system call accepts a client connection.
		   It blocks the server until a client request comes.

		   The accept() system call fills up the client's details
		   in a struct sockaddr which is passed as a parameter.
		   The length of the structure is noted in clilen. Note
		   that the new socket descriptor returned by the accept()
		   system call is stored in "newsockfd".
		*/
	clilen = sizeof(cli_addr);
	newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr,
				&clilen) ;
	if (newsockfd < 0) {
		perror("Accept error\n");
		exit(0);
	}
	// fork
	int fid = fork();
	if(fid == 0){
		// child case
		close(sockfd);
		char username[26],buffer[1000];
		// LOGIN Case
		send(newsockfd,"LOGIN:\0",7,0);
		recv(newsockfd,username,26,0);
		// Finding username
		int found = 0;
		FILE* fp = fopen("users.txt","r");
		if(fp == NULL){
			perror("Unable to open the file");
			exit(1);
		}
		while(fgets(buffer,26,fp)){
			if(buffer[strlen(buffer)-1] == '\n')
				buffer[strlen(buffer)-1] = '\0';
			if(strcmp(buffer,username) == 0){
				found = 1;
				break;
			}
		}
		if(found == 1){
			send(newsockfd,"FOUND\0",6,0);
		} else {
			send(newsockfd,"NOT-FOUND\0",10,0);
		}
		fclose(fp);
		// Command Case
		while(1){
		 	int flag = 1;
		 	char command[1000] = {0};
		 	while(flag){
		 		char buf[11] = {0};
		 		recv(newsockfd,buf,10,0);
		 		for(int i = 0;i < 10;i++)
		 			if(buf[i] == '\n'){
		 				flag = 0;
		 				buf[i] = '\0';
		 				break;
		 			}
		 		strcat(command,buf);
		 	}
		 	// Exit Case
			if(strcmp(buffer,"exit\n") == 0)
				break;
			execute_command(newsockfd,command);
		}
		close(newsockfd);
		exit(0);
	} else
		close(newsockfd);
    }
    close(sockfd);
    return 0; 
}
