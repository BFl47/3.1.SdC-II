#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>  // htons() and inet_addr()
#include <netinet/in.h> // struct sockaddr_in
#include <sys/socket.h>

#include "common.h"
#include "io.h"

int main(int argc, char* argv[]) {

    // variables for handling a socket
    int socket_desc;
    int ret;
    struct sockaddr_in server_addr = {0}; // some fields are required to be filled with 0

    // set up parameters for the connection
    server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
    server_addr.sin_family      = AF_INET;
    server_addr.sin_port        = htons(SERVER_PORT); // don't forget about network byte order!


    /**
     * COMPLETARE QUI
     *
     * Obiettivi:
     * - Craere una stream socket
     * - Inizializzare la connessione lato client
     * - Gestire eventuali errore
     *
     */
    int sockaddr_len = sizeof(struct sockaddr_in);
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1) handle_error("main: socket");

    ret = connect(socket_desc, (struct sockaddr*) &server_addr, (socklen_t) sockaddr_len);
    if (ret) handle_error("main: connect");


    /**
	
	socket_desc = socket(AF_INET, SOCK_STREAM, 0);
	
	ret = connect(socket_desc, &server_addr, sizeof(struct sockaddr_in));
    */

    char buf[MSG_MAX_LENGTH];
    buf[MSG_MAX_LENGTH-1] = '\0';

    size_t msg_len;

    // display welcome message from server
    ret = recv_from_socket(socket_desc, buf, MSG_MAX_LENGTH);
    if(ret == 0) handle_error("Socket closed unexpectedly");
    printf("%s\n",buf);
        // main loop
    while (1) {
        char* quit_command = QUIT_COMMAND;
        size_t quit_command_len = strlen(quit_command);

        memset(buf, 0, sizeof(buf));
        
        printf("Insert your message: ");

        /* Read a line from stdin
         *
         * fgets() reads up to sizeof(buf)-1 bytes and on success
         * returns the first argument passed to it. */
        if (fgets(buf, MSG_MAX_LENGTH-1, stdin) != (char*)buf) {
            fprintf(stderr, "Error while reading from stdin, exiting...\n");
            exit(EXIT_FAILURE);
        }
        msg_len = strlen(buf);
        buf[--msg_len] = '\0';

        send_to_socket(socket_desc, buf);
        if (msg_len == quit_command_len && !memcmp(buf, quit_command, quit_command_len))  break;

        ret = recv_from_socket(socket_desc, buf, MSG_MAX_LENGTH);
        if(ret == 0) handle_error("Socket closed unexpectedly");
        printf("Answer: %s\n",buf);
    }


    /**
     * COMPLETARE QUI
     *
     * Obiettivi:
     * - chiusura della socket
     * - gestire eventuali errore
     *
     */
    ret = close(socket_desc);
    if (ret) handle_error("main: close socket_desk");


    /**
	
	close(socket_desc);
    */

    printf("[Main Thread] Client terminated\n");

    exit(EXIT_SUCCESS);
}
