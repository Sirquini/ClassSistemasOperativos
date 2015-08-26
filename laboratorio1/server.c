#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	pid_t pid;

	pid = fork();
	if (pid == 0)
	{
		/* Child process */
		exit(EXIT_SUCCESS);
	}
	else if (pid > 0)
	{
		/* Parent process */
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
