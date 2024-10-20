#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>

#include "common.h"
#include "io.h"

int send_to_socket(int sd, const char *msg) {

    /**
     * COMPLETARE QUI
     *
     * Obiettivi:
     * - usare il descrittore sd per inviare tutti i byte della stringa msg
     *   (incluso il terminatore di stringa \0)
     * - come valore di ritorno, restituire il numero di byte inviati (incluso
     *   il terminatore di stringa \0)
     * - gestire eventuali interruzioni, riprendendo l'invio da dove era stato
     *   interrotto
     * - gestire eventuali errori, terminando l'applicazione
     *
     */
    int bytes_left = strlen(msg) + 1;
    int ret, sent_bytes = 0;
    

    while (bytes_left > 0) {
        ret = send(sd, msg + sent_bytes, bytes_left, 0);
        if (ret == -1) {
            if (errno == EINTR) continue;
            handle_error("send_to_socket: errore send");
        }
        sent_bytes += ret;
        bytes_left -= ret;
    }
 
    /***/
    return sent_bytes; /*return inserito per permettere la compilazione. Rimuovere*/

}

int recv_from_socket(int sd, char *msg, int max_len) {

    /**
     * COMPLETARE QUI
     *
     * Obiettivi:
     * - usare il descrittore sd per ricevere una stringa da salvare nel buffer
     *   msg (incluso il terminatore di stringa \0); la lunghezza della stringa
     *   non è nota a priori, ma comunque minore di max_len;
     * - ricevere fino ad un massimo di max_len bytes (incluso il terminatore di
     *   stringa \0);
     * - come valore di ritorno, restituire il numero di byte ricevuti (escluso
     *   il terminatore di stringa \0)
     * - in caso di chiusura inaspettata della socket, restituire 0
     * - gestire eventuali interruzioni, riprendendo la ricezione da dove era
     *   stata interrotta
     * - gestire eventuali errori, terminando l'applicazione
     *
     */
    int ret, recv_bytes = 0;
    do {
        ret = recv(sd, msg + recv_bytes, 1, 0);
        if (ret == 0) 
            return 0;
        if (ret == -1) {
            if (errno == EINTR) continue;
            handle_error("recv_from_socket: errore recv");
        }
        recv_bytes += ret;
    } while (msg[recv_bytes-1] != '\0');


    /***/
    return recv_bytes; /*return inserito per permettere la compilazione. Rimuovere*/
}
