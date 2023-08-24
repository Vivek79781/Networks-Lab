// All necessary libraries
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

// Helper Function to check for operator
int isOperator(char ch){
	if(ch == '+'|| ch == '-'|| ch == '*'|| ch == '/')
		return 1;
	return 0;
}

// Helper function for performing operations
double operation(char op,double prev,double next){
	switch(op){
		case '+':
			return prev + next;
		case '-':
			return prev - next;
		case '*':
			return prev*next;
		case '/':
			return prev/next;
		default:
			printf("Incorrect Expression\n");
			return 0;
	}
}

int main()
{
	// Below parts are copied from sample codes
	int			sockfd, newsockfd ; /* Socket descriptors */
	int			clilen;
	struct sockaddr_in	cli_addr, serv_addr;

	int i;
	char str[6];		/* We will use this buffer for communication */

	/* The following system call opens a socket. The first parameter
	   indicates the family of the protocol to be followed. For internet
	   protocols we use AF_INET. For TCP sockets the second parameter
	   is SOCK_STREAM. The third parameter is set to 0 for user
	   applications.
	*/
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
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
	serv_addr.sin_family		= AF_INET;
	serv_addr.sin_addr.s_addr	= INADDR_ANY;
	serv_addr.sin_port		= htons(20100);

	/* With the information provided in serv_addr, we associate the server
	   with its port using the bind() system call. 
	*/
	if (bind(sockfd, (struct sockaddr *) &serv_addr,
					sizeof(serv_addr)) < 0) {
		perror("Unable to bind local address\n");
		exit(0);
	}

	listen(sockfd, 5); /* This specifies that up to 5 concurrent client
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
	while (1) {

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


		/* We initialize the buffer, copy the message to it,
			and send the message to the client. 
		*/
		
		//strcpy(buf,"Message from server");
		//send(newsockfd, buf, strlen(buf) + 1, 0);

		/* We now receive a message from the client. For this example
  		   we make an assumption that the entire message sent from the
  		   client will come together. In general, this need not be true
		   for TCP sockets (unlike UDPi sockets), and this program may not
		   always work (for this example, the chance is very low as the 
		   message is very short. But in general, there has to be some
	   	   mechanism for the receiving side to know when the entire message
		  is received. Look up the return value of recv() to see how you
		  can do this.
		*/

		// My Modifications from Sample Code begins here.
		int l = 1;
		while(l > 0){
			int flag = 0; 	// flag for decimal points as we are sending data in chunks of 5 characters so it can be possible that 1234.1234 is send as 1234. and 1234 so declared here
			double res = 0,next = 0,prev = 0,parathesisOperand = 0,k = 10;	//next number, prev number, and if paranthesis occurs then left will be stored in paranthesisOperand and left = 0
			char currentOperation = '+',parathesisOperator = '+';		//currentOperation to performed and paranthensis Operator 
			l = recv(newsockfd, str, 6, 0);	// receive data of length 5
			while(l > 0){
				int i,j = l-1;		// j is l-1 as when I am sending data of length 5 and \0 at end so if \0 occurs before index 5 then break so l-1 is to take out extra \0
				for(i = 0;i < j;i++){
					if(str[i] == ' ')	// whitespace condition
						continue;
					if(str[i] == '('){	// left paranthesis condition
						parathesisOperand = prev;
						parathesisOperator = currentOperation;
						prev = 0;
						currentOperation = '+';
					} else if (str[i] == ')'){	// right paranthesis condition
						prev = operation(currentOperation,prev,next);
						prev = operation(parathesisOperator,parathesisOperand,prev);
						next = 0;
						currentOperation = '+';
					} else if(isOperator(str[i])){	// is Operator Condition
						prev = operation(currentOperation,prev,next);
						next = 0;
						currentOperation = str[i];
						flag = 0;
					} else if(flag == 0 && str[i] >= '0' && str[i] <= '9'){	// is digit but before decimal point
						next = next*10 + str[i]-'0';
					} else if(flag == 1 && str[i] >= '0' && str[i] <= '9'){	// is degit but after decimal point
						next += (str[i]-'0')/k;
						k *= 10;
					} else if(str[i] == '.'){	// decimal point
						flag = 1;
						k = 10.0;
					}
				}	// ans is stored in prev
				if(str[l-2] == '\n' || str[l-2] == '\0'){	// if l-2 index is \0 or \n as during input we give enter (\n) for next line
					prev = operation(currentOperation,prev,next);
					char ans[11];
					gcvt(prev,10,ans);	// double ans to string
					send(newsockfd, ans , strlen(ans) + 1, 0);	// sending back to client
					break;
				}
				l = recv(newsockfd, str, 6, 0);	// getting from client if incomplete data is send
				
			}
		}
		close(newsockfd);
	}
	return 0;
}
