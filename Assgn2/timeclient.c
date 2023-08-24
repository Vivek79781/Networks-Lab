// A Simple Client Implementation
// 20CS10077	Vivek Jaiswal
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <poll.h>
int main() { 
    int sockfd; 
    struct sockaddr_in servaddr; 
  
    // Creating socket file descriptor 
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if ( sockfd < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
  
    memset(&servaddr, 0, sizeof(servaddr)); 
      
    // Server information 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(8181); 
    inet_aton("127.0.0.1", &servaddr.sin_addr); 
    // Poll defining
    struct pollfd fds[1];
    fds[0].fd = sockfd;
    fds[0].events = POLLIN;
    // declare variables
    int n,retries = 0;
    socklen_t len;
    char buffer[50];
    
    while(retries < 5){
    	// send to server
	sendto(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *) &servaddr, sizeof(servaddr));
	// timeout condition as taught in class
	int pollOut = poll(fds,1,3000);
	// poll conditions
	if(pollOut == 0){
    		retries++;
    	} else if(pollOut > 0){
    		if(fds[0].revents & POLLIN){
    			len = sizeof(servaddr);
    			recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *) &servaddr, &len);
    			printf("Time: %s\n", buffer); 
    			break;
    		}
   	} else {
    		perror("Error in poll");
    		exit(1);
    	}    
    }
    // Time exceeded condition
    if(retries == 5)
    	printf("Time Exceeded\n");
    close(sockfd); 
    return 0; 
} 
