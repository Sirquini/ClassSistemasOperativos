#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <resolv.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <syslog.h>
#define MAXBUF    1024
#define MY_PORT   5050
//WE USED POSTFIX IN LINUX TO SEND EMAILS

int main(int argc, char const *argv[])
{
    int sockfd;
    struct sockaddr_in self;
    char buffer[MAXBUF];

    setlogmask (LOG_UPTO (LOG_INFO | LOG_ERR));
    syslog (LOG_INFO, "Starting Email Service... pid:[%d]", getpid());

    // Create streaming socket
    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
    {
        syslog (LOG_ERR, "[Email Service]: Error in create Socket...");
        exit(errno);
    }

    // Initialize address/port structure
    bzero(&self, sizeof(self));
    self.sin_family = AF_INET;
    self.sin_port = htons(MY_PORT);
    self.sin_addr.s_addr = INADDR_ANY;

    // Assign a port number to the socket
    if ( bind(sockfd, (struct sockaddr*)&self, sizeof(self)) != 0 )
    {
        syslog (LOG_ERR, "[Email Service]: Error in Socket-bind...");
        exit(errno);
    }

    // Make it a listening socket
    if ( listen(sockfd, 20) != 0 )
    {
        syslog (LOG_ERR, "[Email Service]: Error in Socket-listen...");
        exit(errno);
    }

    syslog (LOG_INFO, "[Email Service]: Started...");


    
    //Forever
    while (1)
    {  
        printf("Waiting connection ... \n");
        int clientfd;
        struct sockaddr_in client_addr;
        int addrlen=sizeof(client_addr);

        // accept a connection (creating a data pipe)
        clientfd = accept(sockfd, (struct sockaddr*)&client_addr, &addrlen);
        syslog (LOG_INFO, 
                "[Email Service]: Socket %s:%d connected\n", 
                                            inet_ntoa(client_addr.sin_addr), 
                                            ntohs(client_addr.sin_port));
        recv(clientfd, buffer, MAXBUF, 0);
        syslog (LOG_INFO, "[Email Service]: Data receive [%s]\n", buffer);
        printf("message recibe: %s by %s \n",buffer, inet_ntoa(client_addr.sin_addr));
        //Validate message structure
        //Insert into sqlite
        // int customer_id = 20;
        // char *transaction_id = "QWERTY1234";
        // char *card_number = "9876789545A";
        // int amount = 350000;
        char *dup = strdup(buffer);//Esto hace magia
        char *token = strtok(dup,"|");//Esto separa magicamente el string por espacios
        int c=0;
        char *data[2]; 
        while( token != NULL)//Estos es necesario
        {
            data[c]=token;
            c++;
            token = strtok(NULL,"|");
        }
        printf("Send data: %s to email %s\n",data[0],data[1]);   
    	char *command="";
        asprintf (&command, "echo \"%s\" | mail -s \"detalles compra\" %s",data[0],data[1]);
    	system(command);
    	syslog (LOG_INFO, "[Email Service]: Email [%s] Message\n", data[1]);//,data[0]);
        //Close data connection
        close(clientfd);
    }

    // Clean up (should never get here!)
    close(sockfd);	
	return 0;
}