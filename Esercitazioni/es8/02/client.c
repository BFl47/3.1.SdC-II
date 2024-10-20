#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>  // htons() and inet_addr()
#include <netinet/in.h> // struct sockaddr_in
#include <sys/socket.h>

#include "common.h"

// client UDP
int main(int argc, char* argv[]) {
    int ret, msg_len;;

    // variables for handling a socket
    int socket_desc;
    struct sockaddr_in server_addr = {0}; // some fields are required to be filled with 0
    int sockaddr_len = sizeof(struct sockaddr_in);

    /** [SOLUTION]
     *  TODO: create a socket for contacting the server
     *
     * Suggestions:
     * - protocollo AF_INET
     * - tipo SOCK_STREAM
     */
    socket_desc = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_desc < 0)
        handle_error("Could not create socket");

    if (DEBUG) fprintf(stderr, "Socket created...\n");

    /** [SOLUTION]
     *  TODO: set up parameters for the connection and initiate a connection to the server
     *
     * Suggestions:
     * - set the 3 fields of server:
     * - - server_addr.sin_addr.s_addr: we must specify the server address
     * - - server_addr.sin_family,
     * - - server_addr.sin_port (using htons() method)
     * - initiate a connection to the server
     * - - attention to the bind method:
     * - - it requires as second field struct sockaddr* addr, but our address is a struct sockaddr_in, hence we must cast it (struct sockaddr*) &server_addr
     */
    //
    server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
    server_addr.sin_family      = AF_INET;
    server_addr.sin_port        = htons(SERVER_PORT); // don't forget about network byte order!

    char buf[1024];
    size_t buf_len = sizeof(buf);
    memset(buf,0,buf_len);

    // main loop
    while (1) {
        char* quit_command = SERVER_COMMAND;
        size_t quit_command_len = strlen(quit_command);

        printf("Insert your message: ");

        /* Read a line from stdin
         *
         * fgets() reads up to sizeof(buf)-1 bytes and on success
         * returns the first argument passed to it. */
        if (fgets(buf, sizeof(buf), stdin) != (char*)buf) {
            fprintf(stderr, "Error while reading from stdin, exiting...\n");
            exit(EXIT_FAILURE);
        }

        msg_len = strlen(buf);
//        buf[--msg_len] = '\0'; // remove '\n' from the end of the message
        ret = sendto(socket_desc, buf, msg_len, 0, (struct sockaddr*) &server_addr, sockaddr_len);
        if (ret == -1) handle_error("client: errore sendto");

        if (DEBUG) fprintf(stderr, "Sent message of %d bytes...\n", ret);

        ret = recvfrom(socket_desc, buf, buf_len, 0, (struct sockaddr*) &server_addr, (socklen_t*) &sockaddr_len);
        if (ret == -1) handle_error("client: errore recvfrom");

        if (DEBUG) fprintf(stderr, "Received message of %d bytes...\n", ret);

		if (msg_len == quit_command_len && !memcmp(buf, quit_command, quit_command_len)){

            if (DEBUG) fprintf(stderr, "Sent QUIT command ...\n");
             break;
        }

        printf("Server response: %s\n", buf); 
    }

    /** [SOLUTION]
     *  TODO: close socket and release unused resources
     */
    ret = close(socket_desc);
    if (ret < 0) handle_error("Cannot close the socket");

    if (DEBUG) fprintf(stderr, "Socket closed...\n");

    if (DEBUG) fprintf(stderr, "Exiting...\n");

    exit(EXIT_SUCCESS);
}
