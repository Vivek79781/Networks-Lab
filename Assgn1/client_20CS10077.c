
/*    THE CLIENT PROCESS */

// All Libraries are included
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
	// This are parts of sample code given by AG sir
	int			sockfd ;
	struct sockaddr_in	serv_addr;

	int i;
	char str[6];

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
	serv_addr.sin_port	= htons(20100);

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
	// My code starts here
	while(1){	// While loop for all expressions until terminated
		fgets(str, 6, stdin);	// Get Input of size 5 from stdin
		send(sockfd, str, strlen(str) + 1, 0);	// send input to server
		if(strlen(str) == 3 && str[0] == '-' && str[1] == '1')	// as fgets send "-1" as "-1 " so length will be 3 for termination
			break;
		int flag = 1;	// flag for completion of expression
		while(flag){	// while loop until complete expression is read
			flag = 1;
			for(int i = 0;i < 5;i++)
				if(str[i] == '\n'){	// expression is completed now display the output by server and flag = 0
					char ans[11];
					recv(sockfd, ans, 11, 0);
					printf("%s\n",ans);
					flag = 0;
					break;
				}
			if(flag){	// If not completed  then read and send to server
				fgets(str, 6, stdin);
				send(sockfd, str, strlen(str) + 1, 0);
			}
		}
	}
	close(sockfd);
	return 0;

}

