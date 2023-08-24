/* VIVEK JAISWAL 20CS10077 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <poll.h>
			/* THE SERVER PROCESS */

int get_server_load(struct sockaddr_in serv_addr){
	char buffer[100];
	int server_sockfd;
	if ((server_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Cannot create socket\n");
		exit(0);
	}
	
	if ((connect(server_sockfd, (struct sockaddr *) &serv_addr,
						sizeof(serv_addr))) < 0) {
		perror("Unable to connect to server\n");
		exit(0);
	}
	send(server_sockfd, "SEND LOAD",10,0);
	recv(server_sockfd, buffer, 100, 0);
	printf("Load received from %s port %d: %s\n",inet_ntoa(serv_addr.sin_addr),ntohs(serv_addr.sin_port),buffer);
	close(server_sockfd);
	return atoi(buffer);
}

int main(int argc, char* argv[])
{
	if(argc < 4){
		fprintf(stderr, "ERROR, No Port is Provided\n");
		exit(1);
	}
	int			current_sockfd, client_sockfd, server_sockfd ; /* Socket descriptors */
	int			clilen;
	struct sockaddr_in	cli_addr, lb_addr, serv1_addr, serv2_addr;

	int i;
	char buffer[1000],buf[100];		/* We will use this buffer for communication */

	/* The following system call opens a socket. The first parameter
	   indicates the family of the protocol to be followed. For internet
	   protocols we use AF_INET. For TCP sockets the second parameter
	   is SOCK_STREAM. The third parameter is set to 0 for user
	   applications.
	*/
	if ((current_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Cannot create socket\n");
		exit(0);
	}

	/* The structure "sockaddr_in" is defined in <netinet/in.h> for the
	   internet family of protocols. This has three main fields. The
 	   field "sin_family" specifies the family and is therefore AF_INET
	   for the internet family. The field "sin_addr" specifies the
	   internet address of the server. This field is set to INADDR_ANY
	   for machines having a single IP address. The field "sin_port"
	   specifies the port number of the server.
	*/
	int lb_port_number = atoi(argv[1]);
	lb_addr.sin_family		= AF_INET;
	lb_addr.sin_addr.s_addr	= INADDR_ANY;
	lb_addr.sin_port		= htons(lb_port_number);
	
	int serv1_port_number = atoi(argv[2]);
	serv1_addr.sin_family	= AF_INET;
	inet_aton("127.0.0.1", &serv1_addr.sin_addr);
	serv1_addr.sin_port	= htons(serv1_port_number);
	
	int serv2_port_number = atoi(argv[3]);
	serv2_addr.sin_family	= AF_INET;
	inet_aton("127.0.0.1", &serv2_addr.sin_addr);
	serv2_addr.sin_port	= htons(serv2_port_number);
	
	/* With the information provided in serv_addr, we associate the server
	   with its port using the bind() system call. 
	*/
	if (bind(current_sockfd, (struct sockaddr *) &lb_addr,
					sizeof(lb_addr)) < 0) {
		perror("Unable to bind local address\n");
		exit(0);
	}

	listen(current_sockfd, 5); /* This specifies that up to 5 concurrent client
			      requests will be queued up while the system is
			      executing the "accept" system call below.
			   */

	/* In this program we are illustrating an iterative server -- one
	   which handles client connections one by one.i.e., no concurrency.
	   The accept() system call returns a new socket descriptor
	   which is used for communication with the server. After the
	   communication is over, the process comes back to wait again on
	   the original socket descriptor.
	*/
	
	int load_server[2] = {0};
	int timeout = 5000;
	struct pollfd fds[1];
	fds[0].fd = current_sockfd;
	fds[0].events = POLLIN;
	
	while (1) {
		clilen = sizeof(cli_addr);
		time_t start_time;
		time(&start_time);
		int pollOut = poll(fds,1,timeout);
		if(pollOut <= 0){
			timeout = 5000;
			load_server[0] = get_server_load(serv1_addr);
			load_server[1] = get_server_load(serv2_addr);
		}
		else {
		if(fds[0].revents == POLLIN){
			client_sockfd = accept(current_sockfd, (struct sockaddr *) &cli_addr, &clilen);
			if(client_sockfd < 0){
				perror("Accept error\n");
				exit(0);
			}
			
			if(fork()==0){
				close(current_sockfd);
				for(i = 0;i < 100;i++)		buf[i] = '\0';
				struct sockaddr_in serv_addr;
				int server_port_number;
				if(load_server[0] < load_server[1]){
					serv_addr = serv1_addr;
					server_port_number = serv1_port_number;
				} else {
					serv_addr = serv2_addr;
					server_port_number = serv2_port_number;
				}
				if ((server_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
					perror("Cannot create socket\n");
					exit(0);
				}
				if ((connect(server_sockfd, (struct sockaddr *) &serv_addr,
									sizeof(serv_addr))) < 0) {
					perror("Unable to connect to server\n");
					exit(0);
				}
				printf("Sending Client Request to %s port %d\n",inet_ntoa(serv_addr.sin_addr),ntohs(serv_addr.sin_port));
				send(current_sockfd, "SEND TIME",10,0);
				for(int i = 0;i < 1000;i++)	buffer[i] = '\0';
				int length;
				while(length = recv(server_sockfd,buf,10,0) > 0){
					strcat(buffer,buf);
					if(buf[length - 1] == '\n')
						break;
				}
				close(server_sockfd);
				send(client_sockfd, buffer, strlen(buffer)+1,0);
				close(client_sockfd);
				exit(0);
			}
			time_t end_time;
			time(&end_time);
			double timedifference = difftime(end_time,start_time);
			timeout = timeout - (int)(timedifference*1000);
			if(timeout <= 0)
				timeout = 5000;
		}
		}
		close(client_sockfd);
	}
	return 0;
}
			

