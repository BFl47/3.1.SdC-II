#include <string.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>       // nanosleep()
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <signal.h>
#include "common.h"

struct cell{
    int reward;
    int input;
};

// definizione struttura memoria
struct shared_memory {
     /**
     * COMPLETARE QUI
     *
     * Obiettivi:
     * - definire i semafori unnamed necessari per gestire la concorrenza
     * - devono essere gli stessi (stesso ordine) in producer.c (fare copia/incolla)
     */
    struct cell buf [BUFFER_SIZE];
    int read_index;
    int write_index;
    sem_t sem_empty;
    sem_t sem_full;
    sem_t sem_cs;
};

//definizione shared memory
struct shared_memory *myshm_ptr;
int fd_shm;

void initMemory() {
     /**
     * COMPLETARE QUI
     *
     * Obiettivi:
     * - richiedere al kernerl di creare una memoria condivisa (nome definito in common.h)
     * - configurare la sua dimensione per contenere la struttura struct shared_memory
     * - mappare la memoria condivisa nel puntatore myshm_ptr 
     * - inizializzare la memoria a 0
     * - Gestire gli errori.
     *
     */  
    int size = sizeof(struct shared_memory), ret;

    shm_unlink(SH_MEM_NAME);
    fd_shm = shm_open(SH_MEM_NAME, O_CREAT | O_RDWR, 0666);
    if (fd_shm == -1) handle_error("initMemory: shm_open");

    ret = ftruncate(fd_shm, size);
    if (ret) handle_error("initMemory: ftruncate");

    myshm_ptr = mmap(0, size, PROT_WRITE|PROT_READ, MAP_SHARED, fd_shm, 0);
    if (myshm_ptr == MAP_FAILED) handle_error("initMemory: mmap");

    memset(myshm_ptr, 0, size);

}

void openSemaphores() {
     /**
     * COMPLETARE QUI
     *
     * Obiettivi:
     * - inizializzare i semafori 
     *   ATTENZIONE: i semafori sono condivisi tra processi che non sono legati da parentela (fork)
     *   non lo abbiamo visto nelle esercitazioni, ma ho comunque detto che c'è un parametro
     *   da settare opportunamente in questo caso
     * - gestire gli errori
     */
    int ret;
    ret = sem_init(&myshm_ptr->sem_empty, 1, BUFFER_SIZE);
    if (ret) handle_error("openSemaphores: sem_init empty");

    ret = sem_init(&myshm_ptr->sem_full, 1, 0);
    if (ret) handle_error("openSemaphores: sem_init full");

    ret = sem_init(&myshm_ptr->sem_cs, 1, 1);
    if (ret) handle_error("openSemaphores: sem_init cs");
    
}

void close_everything() {
    /** 
     * COMPLETARE QUI
     *
     * Obiettivi:
     * - chiudere la memoria condivisa
     * - chiedere al kernel di eliminare la memoria condivisa
     * - gestire la chiusura dei semafori
     * - gestire gli errori 
     **/
    int ret, size = sizeof(struct shared_memory);
    ret = sem_destroy(&myshm_ptr->sem_empty);
    if (ret) handle_error("close_everything: sem_destroy empty");
    ret = sem_destroy(&myshm_ptr->sem_full);
    if (ret) handle_error("close_everything: sem_destroy full");
    ret = sem_destroy(&myshm_ptr->sem_cs);
    if (ret) handle_error("close_everything: sem_destroy cs");

    ret = munmap(myshm_ptr, size);
    if (ret) handle_error("close_everything: munmap");
    ret = close(fd_shm);
    if (ret) handle_error("close everything: close fd_shm");
    ret = shm_unlink(SH_MEM_NAME);
    if (ret)  handle_error("close everything: unlink");

}



void consume(){
    int numOps = 0;
    int totalreward = 0;
    while (1) {
        printf("ready to read an element\n");fflush(stdout);

        /** 
         * COMPLETARE QUI
         *
         * L'elaborator preleva un elemento dal buffer e lo elabora (simulato con una pausa) 
         * Occasionalmente stampa quanto ha guadagnato da tutti i task
         * 
         * Obiettivi:
         * - gestire la sezione critica opportunamente tramite i semafori
         * - gestire gli errori 
         **/
        int ret = sem_wait(&myshm_ptr->sem_full);
        if (ret) handle_error("consume: sem_wait full");

        // inizio sezione critica
        printf("reading an element\n");fflush(stdout);
        struct cell value = myshm_ptr->buf[myshm_ptr->read_index];
        if (myshm_ptr->read_index == BUFFER_SIZE-1)
            myshm_ptr->read_index = 0;
        else
            myshm_ptr->read_index++;
        // fine sezione critica    

        ret = sem_post(&myshm_ptr->sem_empty);
        if (ret) handle_error("consume: sem_post empty");

        printf("Elaborating value %d\n",value.input);
        struct timespec pause = {0};
        pause.tv_sec = 2;
        nanosleep(&pause, NULL);

        totalreward += value.reward;
        numOps++;
        if (numOps%10 == 0)
            printf("Total server reward: %d\n", totalreward);
    }
}

/* Signal Handler for SIGINT */
void sigintHandler(int sig_num)
{
    printf("\n SIGINT or CTRL-C detected. Exiting gracefully \n");
    close_everything();
    fflush(stdout);
    exit(0);
}

int main(int argc, char** argv) {

    /* Set the SIGINT (Ctrl-C) signal handler to sigintHandler
       Refer http://en.cppreference.com/w/c/program/signal */
    signal(SIGINT, sigintHandler);
    printf("creating shared memory\n");fflush(stdout);
    initMemory();
    printf("opening semaphores\n");fflush(stdout);
    openSemaphores();

    consume();
    //we never reach this point
    exit(EXIT_SUCCESS);
}
