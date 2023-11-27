#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <semaphore.h>
#include <time.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdbool.h>
#include <ctype.h>
#include <signal.h>
#include <pthread.h>

#define PENSA 0
#define AFFAMATO 1 // prende una forchetta
#define MANGIA 2
#define NAMESHM "shm-filosofi"
#define NAMESTATO "vettore-stati"
#define NAMESEMAFORI "shm-semafori"
#define STOP "stop"

struct sharedMemory
{
	pid_t pidParent;
	int n_pro;
	int deadlock;
	char msg[256];
	int shm_size;
	pid_t pidControllore;
} *shm;

int main()
{

	int shm_fd=0;
	struct sharedMemory *shm;
	shm_fd = shm_open(NAMESHM, O_RDWR, 0666);
	if (shm_fd == -1)
	{
		perror("Errore apertura memoria condivisa");
		exit(EXIT_FAILURE);
	}
	int shmStato_fd = shm_open(NAMESTATO, O_RDWR, 0666);
	if (shmStato_fd == -1)
	{
		perror("Errore apertura memoria condivisa");
		exit(EXIT_FAILURE);
	}
	int shmSem_fd = shm_open(NAMESEMAFORI, O_RDWR, 0666);
	if (shmSem_fd == -1)
	{
		perror("Errore apertura memoria condivisa");
		exit(EXIT_FAILURE);
	}
	shm = (struct sharedMemory *)mmap(NULL, sizeof(struct sharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	printf("shared memory aperta\nmsg: %s\n", shm->msg);
	shm->pidControllore = getpid();
	int *shmStato_ptr = (int *)mmap(NULL, sizeof(int)*shm->n_pro, PROT_READ, MAP_SHARED, shmStato_fd, 0);

	sem_t *forchette = (sem_t *)mmap(NULL, sizeof(sem_t)*shm->n_pro, PROT_READ, MAP_SHARED, shmSem_fd, 0);

	const int n_pro = shm->n_pro;
	int deadlock = n_pro;
	int value = 0;
	int secondiTrasc = 0;
	int countSecStarvation[n_pro];
	for (int i = 0; i < n_pro; i++)
	{
		countSecStarvation[i] = 0;
	}
	
	while(deadlock){
		printf("secondi trascorsi: %d\n\n", secondiTrasc);
		printf("%s\n", shm->msg);
		for(int i = 0; i<n_pro; i++){
			sem_getvalue(&forchette[i], &value);
			printf("valore semaforo %d : %d\n", i, value);
		}
		shmStato_ptr = (int *)mmap(NULL, sizeof(int)*shm->n_pro, PROT_READ, MAP_SHARED, shmStato_fd, 0);
		deadlock = n_pro;
		for (int i = 0; i < shm->n_pro; i++)
		{
			
			printf("%d stato: %d\n", i, shmStato_ptr[i]);
			if(shmStato_ptr[i]==1) {
				deadlock--; 
				countSecStarvation[i]++;
				if(countSecStarvation[i]>20){
					printf("filosofo %d starvation +20sec\n", i);
					kill(shm->pidParent, SIGUSR1);
					exit(-1);
				}
				} else countSecStarvation[i]=0;
		}
		sleep(1);
		secondiTrasc++;
		//system("clear");
		}
		
		printf("\nDeadlock\n");
		munmap(shmStato_ptr, sizeof(int)*shm->n_pro);
		munmap(shm, sizeof(struct sharedMemory));

		 
	
	return 0;
}