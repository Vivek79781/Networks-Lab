// 20CS10077	Vivek Jaiswal
/*    THE CLIENT PROCESS */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main()
{
	int			sockfd ;
	struct sockaddr_in	serv_addr;

	int i;
	char buf[50];

	/* Opening a socket is exactly similar to the server process */
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Unable to create socket\n");
		exit(0);
	}

	/* Recall that we specified INADDR_ANY when we specified the server
	   address in the server. Since the client can run on a different
	   machine, we must specify the IP address of the server. 

	   In this program, we assume that the server is running on the
	   same machine as the client. 127.0.0.1 is a special address
	   for "localhost" (this machine)
	   
	/* IF YOUR SERVER RUNS ON SOME OTHER MACHINE, YOU MUST CHANGE 
           THE IP ADDRESS SPECIFIED BELOW TO THE IP ADDRESS OF THE 
           MACHINE WHERE YOU ARE RUNNING THE SERVER. 
    	*/

	serv_addr.sin_family	= AF_INET;
	inet_aton("127.0.0.1", &serv_addr.sin_addr);
	serv_addr.sin_port	= htons(8010);

	/* With the information specified in serv_addr, the connect()
	   system call establishes a connection with the server process.
	*/
	if ((connect(sockfd, (struct sockaddr *) &serv_addr,
						sizeof(serv_addr))) < 0) {
		perror("Unable to connect to server\n");
		exit(0);
	}

	/* After connection, the client can send or receive messages.
	   However, please note that recv() will block when the
	   server is not sending and vice versa. Similarly send() will
	   block when the server is not receiving and vice versa. For
	   non-blocking modes, refer to the online man pages.
	*/
	for(int i = 0;i < 50;i++) buf[i] = '\0';
	recv(sockfd, buf, 10, 0);
	printf("%s", buf);
	char *username=NULL;
	int size = 0;
	getline(&username,&size,stdin);
	username[strlen(username)-1] = '\0';
	send(sockfd,username,strlen(username)+1,0);
	//printf("1");
	recv(sockfd,buf,10,0);
	//printf("%s\n",buf);
	if(strcmp(buf,"NOT-FOUND") == 0){
		printf("User Not Exist\n");
		close(sockfd);
		exit(1);
	}

	printf("LOGIN SUCCESSFUL\n");
	while(1){
		char *line = NULL;
		printf("Enter Your Command:");
		size = 0;
		getline(&line,&size,stdin);
		//printf("%s",line);
		for(int i = 0;i < strlen(line)+1;i+=50){
			send(sockfd,line+i,(strlen(line)+1-i < 50)?strlen(line)+1-i:50,0);
		}
		line[strlen(line) - 1] = '\0';
		//printf("SEND SUCCESSFULLY\n");
		if(strcmp(line,"exit") == 0){
			close(sockfd);
			exit(0);
		}
		int flag = 1;
		while(flag){
			int l = recv(sockfd,buf,50,0);
			for(int i = 0;i < l;i++){
				if(buf[i] == '\n'){
					flag = 0;
					break;
				}
			}
			if(strcmp(buf,"$$$$\n") == 0)
				printf("Invalid Command\n");
			else if(strcmp(buf,"####\n") == 0)
				printf("Error in running Command\n");
			else
				printf("%s",buf);
		}
		
	}
	close(sockfd);
	return 0;

}

