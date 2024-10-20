#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <semaphore.h>

#define N 100 // number of threads
#define M 1000 // number of iterations per thread
#define V 1 // value added to the balance by each thread at each iteration

unsigned long int shared_variable;
int n = N, m = M, v = V;
sem_t sem;

void* thread_work(void *arg) {
	int i;
	for (i = 0; i < m; i++) {
		if(sem_wait(&sem) != 0) perror("errore sem_wait\n");				// wait for the semaphore
		shared_variable += v;
		if(sem_post(&sem) != 0) perror("errore sem_post\n");				// release the semaphore
	}
}

int main(int argc, char **argv)
{
	if (argc > 1) n = atoi(argv[1]);
	if (argc > 2) m = atoi(argv[2]);
	if (argc > 3) v = atoi(argv[3]);
	shared_variable = 0;

	printf("Initializing the semaphore..."); fflush(stdout);
	if (sem_init(&sem, 0, 1) != 0) perror("errore sem_init");

	printf("Going to start %d threads, each adding %d times %d to a shared variable initialized to zero...", n, m, v); fflush(stdout);
	pthread_t* threads = (pthread_t*)malloc(n * sizeof(pthread_t));
	int i;
	for (i = 0; i < n; i++)
		if (pthread_create(&threads[i], NULL, thread_work, NULL) != 0) {
			fprintf(stderr, "Can't create a new thread, error %d\n", errno);
			exit(EXIT_FAILURE);
		}
	printf("ok\n");

	printf("Waiting for the termination of all the %d threads...", n); fflush(stdout);
	for (i = 0; i < n; i++)
		pthread_join(threads[i], NULL);
	printf("ok\n");

	unsigned long int expected_value = (unsigned long int)n*m*v;
	printf("The value of the shared variable is %lu. It should have been %lu\n", shared_variable, expected_value);
	if (expected_value > shared_variable) {
		unsigned long int lost_adds = (expected_value - shared_variable) / v;
		printf("Number of lost adds: %lu\n", lost_adds);
	}

    free(threads);

	printf("Destroying the semaphore...\n"); fflush(stdout);
	if (sem_destroy(&sem) != 0) perror("errore sem_destroy");

	return EXIT_SUCCESS;
}

