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

#define PORTNO		4040       // Port to bind
#define BUFF_SIZE	1024        // Max size for the string buffer

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
        validator_service();
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
