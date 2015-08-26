#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>

#define PORTNO		4040       // Port to bind
#define BUFF_SIZE	256        // Max size for the string buffer

int main(int argc, char *argv[])
{
	pid_t pid;
	int sockfd, newsockfd, clilen, n_chars;
	char buffer[BUFF_SIZE];
	struct sockaddr_in serv_addr, cli_addr;

	pid = fork();
	if (pid == 0)
	{
		/* Child process */
		exit(EXIT_SUCCESS);
	}
	else if (pid > 0)
	{
		/* Parent process */

		// Create the socket
		sockfd = socket(AF_INET, SOCK_STREAM, 0);

		// Check the socket creation
		if (sockfd < 0)
			perror("socket");

		// Initialize the server address to 0s
		bzero((char *) &serv_addr, sizeof(serv_addr));
		// Set the address family
		serv_addr.sin_family = AF_INET;
		// Set the server port
		serv_addr.sin_port = htons(PORTNO);
		// Set the ip address to the machine ip address
		serv_addr.sin_addr.s_addr = INADDR_ANY;



		exit(EXIT_SUCCESS);
	}
	else
	{
		/* Fork Failure */
		perror("fork");
		exit(EXIT_FAILURE);
	}
	return 0;
}
