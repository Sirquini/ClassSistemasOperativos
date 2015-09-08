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
#include <sqlite3.h>

////////////////////////////////////////////////////////////////////////////////
// Global Variables
////////////////////////////////////////////////////////////////////////////////
#define MY_PORT   4040
#define MAXBUF    1024

////////////////////////////////////////////////////////////////////////////////
// Enumerations
////////////////////////////////////////////////////////////////////////////////
enum codes 
{
    OK,
    NOT_ACCEPTABLE,
    INTERNAL_SERVER_ERROR
};

////////////////////////////////////////////////////////////////////////////////
// Functions (Helpers)
////////////////////////////////////////////////////////////////////////////////
static inline char *get_response_code(enum codes code)
{
    char *strings[] = { "202", "406", "500"};

    return strings[code];
}

int insert_request(int customer_id, 
                   char *transaction_id,
                   char *card_number, 
                   int amount)
{
    sqlite3 *conn;
    sqlite3_stmt    *res;
    int error = 0;
    int rec_count = 0;
    const char *errMSG;
    const char *tail;
    sqlite3_stmt *stmt; 

    setlogmask (LOG_UPTO (LOG_INFO | LOG_ERR));

    error = sqlite3_open("purchase_service.db", &conn);
    if (error) 
    {
        syslog (LOG_ERR, "[Validator Service]: Error can not open database...");
        return -1;
    }

    sqlite3_prepare_v2(conn, 
                       "insert into requests (customer_id, transaction_id, \
                                              card_number, amount) \
                        values (?1, ?2, ?3, ?4);", -1, &stmt, NULL);

    sqlite3_bind_int(stmt, 1, customer_id);
    sqlite3_bind_text(stmt, 2, transaction_id, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, card_number, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, amount);

    int rc = sqlite3_step(stmt); 
    if (rc != SQLITE_DONE) 
    {
        syslog (LOG_ERR, 
                "[Validator Service]: Error inserting data %s\n...", 
                                                    sqlite3_errmsg(conn));
        return -1;
    }
    syslog (LOG_INFO, "[Validator Service] new request inserted");

    sqlite3_finalize(stmt);
    sqlite3_close(conn);

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Validator Service
////////////////////////////////////////////////////////////////////////////////
void validator_service(void)
{
    setlogmask (LOG_UPTO (LOG_INFO | LOG_ERR));
    syslog (LOG_INFO, "Starting Validator Service... pid:[%d]", getpid());

    while(1){
        sqlite3 *conn;
        sqlite3_stmt    *res;
        sqlite3 *conn2;
        sqlite3_stmt    *res2;
        int error = 0;
        int error2 = 0;
        int rec_count = 0;
        const char *errMSG;
        const char *tail;

        error = sqlite3_open("purchase_service.db", &conn);
        if (error) 
        {
            puts("Can not open database");
            exit(0);
        }

        error = sqlite3_prepare_v2(conn,
                                   "select card_number,amount from requests where status = 0",
                                   1000, &res, &tail);
        
        // if (error != SQLITE_OK) 
        // {
        //     puts("We did not get any data!");
        //     exit(0);
        // }
        
        // puts("==========================");
        
        while (sqlite3_step(res) == SQLITE_ROW) 
        {
            error = sqlite3_prepare_v2(conn2,
                                   "select card_number,amount from requests where status = 0",
                                   1000, &res2, &tail);
            printf("%s|", sqlite3_column_text(res, 0));
            printf("%s|", sqlite3_column_text(res, 1));
            printf("%s|", sqlite3_column_text(res, 2));
            printf("%u\n", sqlite3_column_int(res, 3));
            rec_count++;
        }
        
        // puts("==========================");
        // printf("We received %d records.\n", rec_count);
        
        sqlite3_close(conn);
    }

    syslog (LOG_INFO, "[Validator Service]: Started...");
}

////////////////////////////////////////////////////////////////////////////////
// Listener Service
////////////////////////////////////////////////////////////////////////////////
void listener_service(void)
{
    enum codes code = OK;
    int sockfd;
    struct sockaddr_in self;
    char buffer[MAXBUF];

    setlogmask (LOG_UPTO (LOG_INFO | LOG_ERR));
    syslog (LOG_INFO, "Starting Listener Service... pid:[%d]", getpid());

    // Create streaming socket
    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
    {
        syslog (LOG_ERR, "[Listener Service]: Error in create Socket...");
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
        syslog (LOG_ERR, "[Listener Service]: Error in Socket-bind...");
        exit(errno);
    }

    // Make it a listening socket
    if ( listen(sockfd, 20) != 0 )
    {
        syslog (LOG_ERR, "[Listener Service]: Error in Socket-listen...");
        exit(errno);
    }

    syslog (LOG_INFO, "[Listener Service]: Started...");

    //Forever
    while (1)
    {  
        printf("Server waiting connection ... \n");
        int clientfd;
        struct sockaddr_in client_addr;
        int addrlen=sizeof(client_addr);

        // accept a connection (creating a data pipe)
        clientfd = accept(sockfd, (struct sockaddr*)&client_addr, &addrlen);
        syslog (LOG_INFO, 
                "[Listener Service]: Socket %s:%d connected\n", 
                                            inet_ntoa(client_addr.sin_addr), 
                                            ntohs(client_addr.sin_port));

        recv(clientfd, buffer, MAXBUF, 0);
        syslog (LOG_INFO, "[Listener Service]: Data receive [%s]\n", buffer);
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
        char *data[10]; 
        while( token != NULL)//Estos es necesario
        {
            data[c]=token;
            c++;
            token = strtok(NULL,"|");
        }
        if (c==4)
        {
            int customer_id = atoi(data[0]);//20;
            char *transaction_id = data[1];//"QWERTY1234";
            char *card_number = data[2];//"9876789545A";
            int amount = atoi(data[3]);//350000;
            if ( insert_request(customer_id, 
                                transaction_id, 
                                card_number, 
                                amount) == 0 )
                code = OK; //Back response 202 (Received)
            else
                code = INTERNAL_SERVER_ERROR; //Back response 500 
                                              // (Internal server error)

            char *str_code = get_response_code(code);
            send(clientfd, str_code, strlen(str_code), 0);
            printf("response %s\n", get_response_code(code));
        }
        else{
           code = NOT_ACCEPTABLE;
           char *str_code = get_response_code(code);
           send(clientfd, str_code, strlen(str_code), 0);
           printf("response %s\n", get_response_code(code)); 
        }
        //Close data connection
        close(clientfd);   
    }

    // Clean up (should never get here!)
    close(sockfd);
}


////////////////////////////////////////////////////////////////////////////////
// Main function (Service)
////////////////////////////////////////////////////////////////////////////////
int main(int Count, char *Strings[])
{
    enum codes code = OK;
    setlogmask (LOG_UPTO (LOG_INFO | LOG_ERR));

    if (fork() == 0) 
    {
        //Child
        validator_service();
    }
    else 
    {
        //Parent
        listener_service();
    }
    return 0;
}