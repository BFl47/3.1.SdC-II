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

#define SHM_SIZE sizeof(struct shared_memory)

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
    
    sem_t empty_sem;
    sem_t fill_sem;
    sem_t write_sem;
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
     int ret;
     
     shm_unlink(SH_MEM_NAME);
     fd_shm = shm_open(SH_MEM_NAME, O_CREAT | O_RDWR, 0666);
     if (fd_shm == -1) handle_error("initMemory: shm_open");

     ret = ftruncate(fd_shm, SHM_SIZE);
     if (ret) handle_error("initMemory: ftruncate");

     myshm_ptr = mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd_shm, 0);
     if (myshm_ptr == MAP_FAILED) handle_error("initMemory: mmap");

     memset(myshm_ptr, 0, SHM_SIZE);
     
     
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
    ret = sem_init(&myshm_ptr->empty_sem, 1, BUFFER_SIZE);
    if (ret) handle_error("openSem: sem_init empty");

    ret = sem_init(&myshm_ptr->fill_sem, 1, 0);
    if (ret) handle_error("openSem: sem_init fill");

    ret = sem_init(&myshm_ptr->write_sem, 1, 1);
    if (ret) handle_error("openSem: sem_init cs");
    
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
	int ret;
    ret = sem_destroy(&myshm_ptr->empty_sem);
    if (ret) handle_error("close_everything: sem_destroy empty");

    ret = sem_destroy(&myshm_ptr->fill_sem);
    if (ret) handle_error("close_everything: sem_destroy fill");

    ret = sem_destroy(&myshm_ptr->write_sem);
    if (ret) handle_error("close_everything: sem_destroy cs");

	ret = munmap(myshm_ptr, SHM_SIZE);
    if (ret) handle_error("close_everything: munmap");

    ret = close(fd_shm);
    if (ret) handle_error("close_everything: close fd_shm");

    ret = shm_unlink(SH_MEM_NAME);
    if (ret) handle_error("close_everything: shm_unlink");

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
        int ret;
        ret = sem_wait(&myshm_ptr->fill_sem);
        if (ret) handle_error("consume: sem_wait fill");
         
		//inizio sezione critica
        printf("reading an element\n");fflush(stdout);
        struct cell value = myshm_ptr->buf[myshm_ptr->read_index];
        if (myshm_ptr->read_index == BUFFER_SIZE-1)
            myshm_ptr->read_index = 0;
        else
            myshm_ptr->read_index++;
        //fine sezione critica    
        
        ret = sem_post(&myshm_ptr->empty_sem);
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
