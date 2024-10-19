#include "common.h"

#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <pthread.h>


// data array
int * data;
/** COMPLETE THE FOLLOWING CODE BLOCK
*
* Add any needed resource 
**/
int shm_fd;
sem_t *sem_req, *sem_wrk;

int request() {
  /** COMPLETE THE FOLLOWING CODE BLOCK
  *
  * map the shared memory in the data array
  **/
  data = (int *)mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  if (data == MAP_FAILED) handle_error("request: mmap");

  printf("request: mapped address: %p\n", data);

  int i;
  for (i = 0; i < NUM; ++i) {
    data[i] = i;
  }
  printf("request: data generated\n");

   /** COMPLETE THE FOLLOWING CODE BLOCK
    *
    * Signal the worker that it can start the elaboration
    * and wait it has terminated
    **/
  if (sem_post(sem_req)) handle_error("request: sem_post req");
  if (sem_wait(sem_wrk)) handle_error("request: sem_wait wrk");

  printf("request: acquire updated data\n");

  printf("request: updated data:\n");
  for (i = 0; i < NUM; ++i) {
    printf("%d\n", data[i]);
  }

   /** COMPLETE THE FOLLOWING CODE BLOCK
    *
    * Release resources
    **/
  munmap(data, SIZE);


  return EXIT_SUCCESS;
}

int work() {

  /** COMPLETE THE FOLLOWING CODE BLOCK
  *
  * map the shared memory in the data array
  **/

  data = (int *)mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  if (data == MAP_FAILED) handle_error("worker: mmap");

  printf("worker: mapped address: %p\n", data);

   /** COMPLETE THE FOLLOWING CODE BLOCK
    *
    * Wait that the request() process generated data
    **/
  if (sem_wait(sem_req)) handle_error("worker: sem_wait req");

  printf("worker: waiting initial data\n");

  printf("worker: initial data acquired\n");

  printf("worker: update data\n");
  int i;
  for (i = 0; i < NUM; ++i) {
    data[i] = data[i] * data[i];
  }

  printf("worker: release updated data\n");

   /** COMPLETE THE FOLLOWING CODE BLOCK
    *
    * Signal the requester that elaboration terminated
    **/
  if (sem_post(sem_wrk)) handle_error("worker: sem_post wrk");


  /** COMPLETE THE FOLLOWING CODE BLOCK
   *
   * Release resources
   **/
  munmap(data, SIZE);

  return EXIT_SUCCESS;
}



int main(int argc, char **argv){

   /** COMPLETE THE FOLLOWING CODE BLOCK
    *
    * Create and open the needed resources 
    **/
    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) handle_error("main: shm_open shm_fd");
    if (ftruncate(shm_fd, SIZE)) handle_error("main: ftruncate");

    sem_req = sem_open(SEM_NAME_REQ, O_CREAT, 0666, 0);
    if (sem_req == SEM_FAILED) handle_error_en(errno, "main: sem_open sem_req");
    sem_wrk = sem_open(SEM_NAME_WRK, O_CREAT, 0666, 0);
    if (sem_wrk == SEM_FAILED) handle_error_en(errno, "main: sem_open sem_wrk");



    int ret;
    pid_t pid = fork();
    if (pid == -1) {
        handle_error("main: fork");
    } else if (pid == 0) {
        work();
        _exit(EXIT_SUCCESS);
    }

    request();
    int status;
    ret = wait(&status);
    if (ret == -1)
      handle_error("main: wait");
    if (WEXITSTATUS(status)) handle_error_en(WEXITSTATUS(status), "request() crashed");


   /** COMPLETE THE FOLLOWING CODE BLOCK
    *
    * Close and release resources
    **/
    close(shm_fd);
    shm_unlink(SHM_NAME);

    if (sem_close(sem_req)) handle_error("main: sem_close sem_req");
    if (sem_unlink(SEM_NAME_REQ)) handle_error("main: sem_unlink sem_req");
    if (sem_close(sem_wrk)) handle_error("main: sem_close sem_wrk");
    if (sem_unlink(SEM_NAME_WRK)) handle_error("main: sem_unlink sem_wrk");


    _exit(EXIT_SUCCESS);

}
