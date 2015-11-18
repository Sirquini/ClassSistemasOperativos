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
int main(int argc, char *argv[])
{
    int clientSocket;
    char buffer[1024];
    struct sockaddr_in serverAddr;
    socklen_t addr_size;

    /*---- Create the socket. The three arguments are: ----*/
    /* 1) Internet domain 2) Stream socket 3) Default protocol (TCP in this case) */
    clientSocket = socket(PF_INET, SOCK_STREAM, 0);

    /*---- Configure settings of the server address struct ----*/
    /* Address family = Internet */
    serverAddr.sin_family = AF_INET;
    /* Set port number, using htons function to use proper byte order */
    serverAddr.sin_port = htons(4040);
    /* Set IP address to localhost */
    if(argv[1]==""){
        printf("Error, ingrese direccion\n");
        exit(1);
    }
    serverAddr.sin_addr.s_addr = inet_addr(argv[1]);
    //////serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    /* Set all bits of the padding field to 0 */
    memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);  

    /*---- Connect the socket to the server using the address struct ----*/
    addr_size = sizeof serverAddr;
    // if(connect(clientSocket, (struct sockaddr *) &serverAddr, addr_size)<0){
    //     printf("Error con la conexion al servidor, intente de nuevo\n");
    //     exit(1);
    // }
    ////while(1){
        if(connect(clientSocket, (struct sockaddr *) &serverAddr, addr_size)<0){
            printf("Error con la conexion al servidor, intente de nuevo\n");
            exit(1);
        }   
        printf("ingrese los datos (CustomerID|TransactionID|CardNumber|Value): ");
        bzero(buffer,256);
        //fgets(buffer,255,stdin);
        scanf("%s",buffer);

        send(clientSocket, buffer,strlen(buffer),0);  

        memset(buffer, '\0', 1024);

        recv(clientSocket, buffer, 1024, 0);

        printf("Data received: %s \n",buffer);   
    ////}
    ////strcpy(buffer,"20|QWERTY1234|9876789545A|350000\n");
    ////send(clientSocket, buffer,strlen(buffer),0);  


    ////memset(buffer, '\0', 1024);
    /*---- Read the message from the server into the buffer ----*/
    ////recv(clientSocket, buffer, 1024, 0);

    /*---- Print the received message ----*/
    ////printf("Data received: %s",buffer);   

    return 0;
}