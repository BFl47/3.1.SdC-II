#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>  // htons()
#include <netinet/in.h> // struct sockaddr_in
#include <sys/socket.h>

#include "common.h"

// server UDP
int main(int argc, char* argv[]) {
    int ret, msg_len;
    char buf[1024];
    size_t buf_len = sizeof(buf);

    int socket_desc;

    // some fields are required to be filled with 0
    struct sockaddr_in server_addr = {0}, client_addr = {0};

    int sockaddr_len = sizeof(struct sockaddr_in); // we will reuse it for accept()

    /** [SOLUTION]
     *  TODO: Create a socket for listening
     *
     * Suggestions:
     * - protocollo AF_INET
     * - tipo SOCK_STREAM
     */
    socket_desc = socket(AF_INET , SOCK_DGRAM , 0);
    if (socket_desc < 0)
        handle_error("Could not create socket");

    if (DEBUG) fprintf(stderr, "Socket created...\n");

    /* We enable SO_REUSEADDR to quickly restart our server after a crash:
     * for more details, read about the TIME_WAIT state in the TCP protocol */
    int reuseaddr_opt = 1;
    ret = setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &reuseaddr_opt, sizeof(reuseaddr_opt));
    if (ret < 0)
        handle_error("Cannot set SO_REUSEADDR option");

    /** [SOLUTION]
     *  TODO: set server address and bind it to the socket
     *
     * Suggestions:
     * - set the 3 fields of server:
     * - - server_addr.sin_addr.s_addr: we want to accept connections from any interface
     * - - server_addr.sin_family,
     * - - server_addr.sin_port (using htons() method)
     * - bind address to socket
     * - - attention to the bind method:
     * - - it requires as second field struct sockaddr* addr, but our address is a struct sockaddr_in, hence we must cast it (struct sockaddr*) &server_addr
     */
    server_addr.sin_addr.s_addr = INADDR_ANY; // we want to accept connections from any interface
    server_addr.sin_family      = AF_INET;
    server_addr.sin_port        = htons(SERVER_PORT); // don't forget about network byte order!

    ret = bind(socket_desc, (struct sockaddr*) &server_addr, sockaddr_len);
    if (ret < 0)
        handle_error("Cannot bind address to socket");

    if (DEBUG) fprintf(stderr, "Binded address to socket...\n");

    /** [SOLUTION]
     *  TODO: start listening
     *
     * Suggestions:
     * - set the number of pending connections to as MAX_CONN_QUEUE
     */
    // loop to handle incoming connections (sequentially)
    while (1) {
		// accept an incoming connection
        /** [SOLUTION]
         *
         * Suggestions:
         * - the descriptor returned by accept() should be stored in the
         *   client_desc variable (declared at the beginning of main)
         * - pass the address of the client_addr variable (casting it to
         *   struct sockaddr* is recommended) as second argument to
         *   accept()
         * - the size of the client_addr structure has been stored in
         *   the sockaddr_len variable, so simply use it! (note that as
         *   the variable is an int, casting its address to socklen_t*
         *   is recommended)
         * - check the return value of accept() for errors!
         */
        memset(buf, 0, buf_len);

        ret = recvfrom(socket_desc, buf, buf_len, 0, (struct sockaddr*) &client_addr, (socklen_t*) &sockaddr_len);
        if (ret == -1) handle_error("server: errore recvfrom");

        if (DEBUG) fprintf(stderr, "Received message of %d bytes...\n", ret);   

        msg_len = strlen(buf);
        ret = sendto(socket_desc, buf, msg_len, 0, (struct sockaddr*) &client_addr, sockaddr_len);
        if (ret == -1) handle_error("server: errore sendto");

        if (DEBUG) fprintf(stderr, "Socket has sent datagram of %d bytes...\n", ret);
        if (DEBUG) fprintf(stderr, "Done!\n");
    }

    exit(EXIT_SUCCESS); // this will never be executed
}
