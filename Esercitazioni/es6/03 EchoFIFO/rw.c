#include <unistd.h>
#include <errno.h>
#include "common.h"


int readOneByOne(int fd, char* buf, char separator) {

    int ret;
    int read_bytes = 0;

    /** [TO DO] READ THE MESSAGE THROUGH THE FIFO DESCRIPTOR
     *
     * Suggestions:
     * - you can read from a FIFO as from a regular file descriptor
     * - since you don't know the length of the message, just
     *   read one byte at a time from the socket
     * - leave the cycle when 'separator' ('\n') is encountered 
     * - repeat the read() when interrupted before reading any data
     * - return the number of bytes read
     * - reading 0 bytes means that the other process has closed
     *   the FIFO unexpectedly: this is an error that should be
     *   dealt with!
     **/
    do {
        ret = read(fd, buf + read_bytes, sizeof(char));
        if (ret == 0) handle_error("rw_readOneByOne: fifo chiuso in modo inaspettato");
        else if (ret == -1) {
            if (errno == EINTR) continue;
            handle_error("rw_readOneByOne: errore in lettura");
        }
        read_bytes++;
    } while (buf[read_bytes - 1] != separator);
    return read_bytes;

}

void writeMsg(int fd, char* buf, int size) {

    int ret;
    int written_bytes = 0;
    /** [TO DO] SEND THE MESSAGE THROUGH THE FIFO DESCRIPTOR
     *
     * Suggestions:
     * - you can write on the FIFO as on a regular file descriptor
     * - make sure that all the bytes have been written: use a while
     *   cycle in the implementation as we did for file descriptors!
     **/
    while (size > 0) {
        ret = write(fd, buf + written_bytes, size);
        if (ret == -1) {
            if (errno == EINTR) continue;
            handle_error("rw_writeMsg: errore in scrittura");
        }
        written_bytes += ret;
        size -= ret;
    }

}
