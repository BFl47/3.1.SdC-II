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
#include "common.h"

// definizione struttura memoria
struct cell{
    int reward;
    int input;
};

struct shared_memory {
     /**
     * COMPLETARE QUI
     *
     * Obiettivi:
     * - definire i semafori unnamed necessari per gestire la concorrenza
     * - devono essere gli stessi (stesso ordine) in elaborator.c (fare copia/incolla)
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


void openMemory() {
     /**
     * COMPLETARE QUI
     *
     * Obiettivi:
     * - richiedere al kernerl l'accesso alla memoria condivisa creata da elaborator.c
     * - Gestire gli errori.
     *
     */    
    int size = sizeof(struct shared_memory);
    fd_shm = shm_open(SH_MEM_NAME, O_RDWR, 0666);
    if (fd_shm == -1) handle_error("openMemory: shm_open");

    myshm_ptr = mmap(0, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd_shm, 0);
    if (myshm_ptr == MAP_FAILED) handle_error("openMemory: mmap");

}

void request(){
    int balance = generateRandomNumber(10000);
    while(1){
        /** 
         * COMPLETARE QUI
         *
         * Il producer genera una serie di input da scrivere nel buffer 
         * Ogni input dovraà essere elaborato dall'elaborator
         * a cui viene corrisposto un pagamento (reward)
         * Quando il pagamento supera la disponibilità il producer termina la
         * propria attività
         * 
         * Obiettivi:
         * - gestire la sezione critica opportunamente tramite i semafori
         * - gestire gli errori 
         **/

        int reward = generateRandomNumber(1000);
        if (reward>balance){
            printf("not enough money, exit\n");fflush(stdout);
            break;
        }
        balance -= reward;

        int input = generateRandomNumber(100);
        printf("Generated input %d\n",input);
        struct timespec pause = {0};
        pause.tv_nsec = 500000000; // 0.5 s (1*10^9 ns)
        nanosleep(&pause, NULL);

        int ret = sem_wait(&myshm_ptr->sem_empty);
        if (ret) handle_error("request: sem_wait empty");
        ret = sem_wait(&myshm_ptr->sem_cs);
        if (ret) handle_error("request: sem_wait cs");

        // inizio critical section
        myshm_ptr->buf[myshm_ptr->write_index].reward = reward;
        myshm_ptr->buf[myshm_ptr->write_index].input = input;
        myshm_ptr->write_index++;
        if (myshm_ptr->write_index == BUFFER_SIZE)
            myshm_ptr->write_index = 0;
        // fine critical section

        ret = sem_post(&myshm_ptr->sem_cs);
        if (ret) handle_error("request: sem_post cs");
        ret = sem_post(&myshm_ptr->sem_full);
        if (ret) handle_error("request: sem_post full");

    }

}

void closeSemaphores() {
    /** 
     * COMPLETARE QUI
     *
     * Obiettivi:
     * - gestire la chiusura dei semafori
     * - gestire gli errori 
     **/
    int ret;
    ret = sem_destroy(&myshm_ptr->sem_empty);
    if (ret) handle_error("close_everything: sem_destroy empty");
    ret = sem_destroy(&myshm_ptr->sem_full);
    if (ret) handle_error("close_everything: sem_destroy full");
    ret = sem_destroy(&myshm_ptr->sem_cs);
    if (ret) handle_error("close_everything: sem_destroy cs");
}

void closeMemory() {
    /** 
     * COMPLETARE QUI
     *
     * Obiettivi:
     * - chiudere la memoria condivisa
     * - chiedere al kernel di eliminare la memoria condivisa
     * - gestire gli errori 
     **/
    int ret, size = sizeof(struct shared_memory);
    ret = munmap(myshm_ptr, size);
    if (ret) handle_error("close_everything: munmap");
    ret = close(fd_shm);
    if (ret) handle_error("close everything: close fd_shm");

}


int main(int argc, char** argv) {
    initRandomGenerator();

    //load memory
    printf("opening shared memory\n");fflush(stdout);
    openMemory();

    //request cycle
    request();

    //close semaphores
    closeSemaphores();

    //close memory
    closeMemory();

    exit(EXIT_SUCCESS);
}

