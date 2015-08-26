#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>

int main(int argc, char *argv[])
{
   int sockfd, portno, n;
   struct sockaddr_in serv_addr;
   struct hostent *server;
   
   char buffer[256];

   portno = 4040;
   
   /* crea el socket */
   sockfd = socket(AF_INET, SOCK_STREAM, 0);
   
   if (sockfd < 0)
   {
      perror("ERROR abriendo socket");
      exit(1);
   }
   server = gethostbyname(argv[0]);
   //server = gethostbyname(argv[0]);  //localhost
   if (server == NULL) {
      fprintf(stderr,"ERROR, no se encontro el host\n");
      exit(0);
   }
   
   bzero((char *) &serv_addr, sizeof(serv_addr));
   serv_addr.sin_family = AF_INET;
   bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
   serv_addr.sin_port = htons(portno);
   
   /* Conectando al servidor */
   if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
   {
      perror("ERROR Conectando");
      exit(1);
   }
   while(1){
       /* mensaje a enviar por el cliente
       * sera leido por el servidor
       */
       printf("ingrese los datos (CustomerID|TransactionID|CardNumber|Value): ");
       bzero(buffer,256);
       fgets(buffer,255,stdin);
       if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
       {
          perror("ERROR Conectando");
          exit(1);
       }
       /* enviar el mensaje al servidor */
       n = write(sockfd, buffer, strlen(buffer));
       
       if (n < 0)
       {
          perror("ERROR enviando el mensaje");
          exit(1);
       }
       
       /* respuesta */
       bzero(buffer,256);
       n = read(sockfd, buffer, 255);
       
       if (n < 0)
       {
          perror("ERROR leyendo respuesta del socket");
          exit(1);
       }
       if(buffer=="202"){
            printf("mensaje recibido!   codigo:%s\n",buffer);
       }
       else{
        printf("mensaje no recibido   codigo:%s\n",buffer);
       }
   }
   return 0;
}