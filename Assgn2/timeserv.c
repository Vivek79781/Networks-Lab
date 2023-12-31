// A Simple UDP Server that sends a HELLO message
// 20CS10077	Vivek Jaiswal
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <time.h>
#define MAXLINE 1024 
  
int main() { 
    int sockfd; 
    struct sockaddr_in servaddr, cliaddr; 
      
    // Create socket file descriptor 
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if ( sockfd < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
      
    memset(&servaddr, 0, sizeof(servaddr)); 
    memset(&cliaddr, 0, sizeof(cliaddr)); 
      
    servaddr.sin_family    = AF_INET; 
    servaddr.sin_addr.s_addr = INADDR_ANY; 
    servaddr.sin_port = htons(8181); 
      
    // Bind the socket with the server address 
    if ( bind(sockfd, (const struct sockaddr *)&servaddr,  
            sizeof(servaddr)) < 0 ) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
    
    printf("\nServer Running....\n");
  
    int n; 
    socklen_t len;
    char buffer[MAXLINE];
    while(1){ 
    	// Copied the Sample Code and previous time server assignment
    	len = sizeof(cliaddr);
        recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct servaddr *) &cliaddr, &len);
    	time_t rawtime;
    	struct tm * timeinfo;
    
    	time ( &rawtime );
    	timeinfo = localtime ( &rawtime );
    	strcpy(buffer,asctime (timeinfo)); 
    	sendto(sockfd, buffer, sizeof(buffer), 0, (struct servaddr *) &cliaddr, len);
    }
    close(sockfd);
    return 0; 
} 
