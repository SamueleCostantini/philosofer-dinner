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
#define STARVATION 20

struct sharedMemory
{
	pid_t pidParent;	  // PID del processo principale (parent)
	int n_pro;			  // Numero di filosofi
	int deadlock;		  // Indicatore di deadlock
	char msg[256];		  // Messaggio per la comunicazione
	int shm_size;		  // Dimensione della shared memory
	bool ril_star;		  // Flag per il rilevamento di starvation
	bool ril_stallo;	  // Flag per il rilevamento di stallo
	pid_t pidControllore; // PID del processo di controllo
} *shm;

void SigIntHandler1(int iSignalCode)
{

	printf("Interrotto, arrivato segnale di stop, n. segnale : %d\n", getpid(), iSignalCode);

	exit(-1);
}

int main()
{

	signal(SIGINT, SigIntHandler1);
	signal(SIGQUIT, SigIntHandler1);
	signal(SIGTERM, SigIntHandler1);
	signal(SIGSTOP, SigIntHandler1);

	sleep(10);
	/*
		secondi di attesa per evitare che rilevi lo stallo nella fase iniziale dl processo parent
		ovvero quando i filosofi sono tutti affamati
	*/

	int shm_fd = 0;
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

	/*preparazione output su filelog*/

	printf("shared memory aperta\nmsg: %s\n", shm->msg);
	shm->pidControllore = getpid(); // comunico il pid del controllore cosi da poterlo terminare una volta terminato il provesso parent
	int *shmStato_ptr = (int *)mmap(NULL, sizeof(int) * shm->n_pro, PROT_READ, MAP_SHARED, shmStato_fd, 0);

	sem_t *forchette = (sem_t *)mmap(NULL, sizeof(sem_t) * shm->n_pro, PROT_READ, MAP_SHARED, shmSem_fd, 0);

	const int n_pro = shm->n_pro;
	int deadlock = n_pro;
	/*
		per rilevare la deadlock setto un contatore con il numero dei filosofi
		i filosofi affamat decrementano il contatore
		se arriva a 0 allora sono tutti affamati e c'è deadlock
		perche tutti hanno una forchetta
	*/
	int value = 0; // variabile per stampare il valore del semaforo
	int secondiTrasc = 0;
	int countSecStarvation[n_pro];
	for (int i = 0; i < n_pro; i++)
	{
		countSecStarvation[i] = 0;
	}
	
	while (deadlock>0)
	{
		printf("secondi trascorsi: %d\n\n", secondiTrasc);
		printf("%s\n", shm->msg);
		for (int i = 0; i < n_pro; i++)
		{
			sem_getvalue(&forchette[i], &value);
			printf("valore semaforo %d : %d\n", i, value);
		}
		shmStato_ptr = (int *)mmap(NULL, sizeof(int) * shm->n_pro, PROT_READ, MAP_SHARED, shmStato_fd, 0);
		deadlock = n_pro;
		for (int i = 0; i < shm->n_pro; i++)
		{
			switch (shmStato_ptr[i])
			{
			case 0:
				printf("%d stato: %s\n", i, "PENSA");
				break;
			case 1:
				printf("%d stato: %s\n", i, "AFFAMATO");
				break;
			case 2:
				printf("%d stato: %s\n", i, "MANGIA");
				break;
			}

			if (shmStato_ptr[i] == 1)
			{
				deadlock--;
				countSecStarvation[i]++;
				if (countSecStarvation[i] > STARVATION)
				{
					printf("filosofo %d starvation +20sec\n", i);

					if (shm->ril_star)
					{
						kill(shm->pidParent, SIGUSR1); //il signal user 1 l'ho utilizzato per segnalare la starvation
						munmap(shmStato_ptr, sizeof(int) * shm->n_pro);
						munmap(shm, sizeof(struct sharedMemory));
						return 3;
					}
					// exit(-1);
				}
			}
			else
				countSecStarvation[i] = 0;
		}
		sleep(1);
		secondiTrasc++; //contatore dei secondi trascorsi per la starvation
		if (deadlock == 0)
		{
			printf("Rilevato uno stallo\n");
			if (shm->ril_stallo)
			{
				kill(shm->pidParent, SIGUSR2); //il signal user 2 l'ho utilizzato per segnalare uno stallo
				munmap(shmStato_ptr, sizeof(int) * shm->n_pro);
				munmap(shm, sizeof(struct sharedMemory));
			}
		}
	}


	return 0;
}