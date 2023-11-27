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
     int n_pro;
     int deadlock;
     char msg[256];
     int shm_size;
};

void SigIntHandler(int iSignalCode)
{
     printf("Ricevuto signal %d\n", iSignalCode);

     struct sharedMemory *shm_ptr;
     int shm_fd = 0;
     if ((shm_fd = shm_open(NAMESHM, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) == -1)
     {
          printf("errore apertura shm \n");
          exit(-1);
     }
     shm_ptr = (struct sharedMemory *)mmap(NULL, sizeof(struct sharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

     sprintf(shm_ptr->msg, STOP);
     printf("%s\n", shm_ptr->msg);

     // shm_unlink(NAMESHM);
}
void SigIntHandler1(int iSignalCode)
{

     printf("Filosofo (%d) riceve signal interrupt, pulisco e chiudo - %d\n", getpid(), iSignalCode);

     exit(0);
}

void filosofo(int i, int sizeShm)
{
     
     signal(SIGINT, SigIntHandler1);

     int shm_fd = 0, shmStato_fd, shmSem_fd;
     
     int *stato;
     if ((shm_fd = shm_open(NAMESHM, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) == -1)
     {
          printf("errore apertura shm - %d\n", errno);
          exit(-1);
     }
     if ((shmStato_fd = shm_open(NAMESTATO, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) == -1)
     {
          printf("errore apertura shm - %d\n", errno);
          exit(-1);
     }
     if ((shmSem_fd = shm_open(NAMESEMAFORI, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) == -1)
     {
          printf("errore apertura shm - %d\n", errno);
          exit(-1);
     }
     struct sharedMemory *shm_ptr = (struct sharedMemory *)mmap(NULL, sizeShm, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
     if (shm_ptr == MAP_FAILED)
     {
          perror("mmap");
          exit(1);
     }
     sem_t *forchette;
     forchette = (sem_t*)mmap(NULL, sizeof(sem_t)*shm_ptr->n_pro, PROT_READ | PROT_WRITE, MAP_SHARED, shmSem_fd, 0);
     if (forchette == MAP_FAILED)
     {
          perror("mmap");
          exit(1);
     }

     int *shmStato_ptr = (int*)mmap(NULL, sizeof(int)*shm_ptr->n_pro, PROT_READ | PROT_WRITE, MAP_SHARED, shmStato_fd, 0);
     if (shmStato_ptr == MAP_FAILED)
     {
          perror("mmap");
          exit(1);
     }
     
     const int n_pro = shm_ptr->n_pro;
     printf("%d. filosofo: pid %d\n", i + 1, getpid());
     printf("valori shm:\n n_pro: %d\n msg: %s\n", shm_ptr->n_pro, shm_ptr->msg);
     
     //random number 1 to 10
      srand(time(NULL)+clock());
     int casualNumber=rand()%10;
     while (1)
     {
          srand(time(NULL)+clock());
          casualNumber=1+rand()%11;
          int j = i+1;
          if(i==n_pro-1)  j=0; 
          
          
          strcpy(shm_ptr->msg, "FILOSOFI MANGIANO\n");
          sleep(rand()%10);
          sem_wait(&forchette[i]);
          printf("%d prende forchetta %d - destra\n", i, i);
          
          shmStato_ptr[i] = AFFAMATO;
          
          //sem_wait(&forchette[j]);
          printf("%d prende forchetta %d - sinistra\n", i, j);
          shmStato_ptr[i] = MANGIA;

          printf("%d Mangia - %dsec\n", i, casualNumber);
          sleep(casualNumber);

          sem_post(&forchette[i]);
          

          //sem_post(&(forchette[j]));
          printf("%d lascia forchetta destra e sinistra\n", i);
     }
     // shm_unlink(NAMESHM);
     strcpy(shm_ptr->msg, "FILOSOFI FINISCONO DI MANGIARE\n");
     munmap(forchette, sizeof(sem_t)*n_pro);
     munmap(shm_ptr, sizeof(shm_ptr));
     munmap(shmStato_ptr,sizeof(int)*n_pro);
     exit(1);
}

void filosofoStallo(int i, int sizeShm)
{
     
     signal(SIGINT, SigIntHandler1);

     int shm_fd = 0, shmStato_fd, shmSem_fd;
     int *stato;
     sem_t *forchette;
     if ((shm_fd = shm_open(NAMESHM, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) == -1)
     {
          printf("errore apertura shm - %d\n", errno);
          exit(-1);
     }
     if ((shmStato_fd = shm_open(NAMESTATO, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) == -1)
     {
          printf("errore apertura shm - %d\n", errno);
          exit(-1);
     }
     if ((shmSem_fd = shm_open(NAMESEMAFORI, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) == -1)
     {
          printf("errore apertura shm - %d\n", errno);
          exit(-1);
     }
     struct sharedMemory *shm_ptr = (struct sharedMemory *)mmap(NULL, sizeShm, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
     if (shm_ptr == MAP_FAILED)
     {
          perror("mmap");
          exit(1);
     }
     int *shmStato_ptr = (int*)mmap(NULL, sizeof(int)*shm_ptr->n_pro, PROT_READ | PROT_WRITE, MAP_SHARED, shmStato_fd, 0);

     forchette = (sem_t*)mmap(NULL, sizeof(sem_t)*shm_ptr->n_pro, PROT_READ | PROT_WRITE, MAP_SHARED, shmSem_fd, 0);

     printf("valori shm:\n n_pro: %d\n msg: %s\n", shm_ptr->n_pro, shm_ptr->msg);
     const int n_pro = shm_ptr->n_pro;

     printf("%d. filosofo: pid %d\n", i + 1, getpid());

     while (1)
     {
          srand(time(NULL)*clock());
          int casualTime = 1+rand()%11;
          int j = rand()%n_pro;
          int k = 0;
          while(k==j){
               k = rand()%n_pro;
          }
          shmStato_ptr[i] = PENSA;
          
          strcpy(shm_ptr->msg, "FILOSOFI MANGIANO\n");
          sem_wait(&forchette[j]);
          printf("%d prende forchetta %d - 1/2\n", i, j);
          shmStato_ptr[i] = AFFAMATO;
          sleep(1);
          sem_wait(&forchette[k]);
          printf("%d prende forchetta %d - 2/2 - mangia\n", i, k);
          shmStato_ptr[i] = MANGIA;
          

          sleep(casualTime);

          sem_post(&forchette[j]);
          printf("%d lascia forchetta %d - 1/2\n", i, j);

          sem_post(&forchette[k]);
          printf("%d lascia forchetta %d - 2/2\n", i, k);
     }
     // shm_unlink(NAMESHM);
     exit(1);
}
int main(int argc, char *argv[])
{
     puts("main");
     const int n_pro = atoi(argv[1]);
     short int ril_stallo = 0, ab_stallo = 0, riv_star = 0;
     sem_t forchette[n_pro];
     int stato[n_pro];
     int deadlock = n_pro;
     srand(time(NULL));
     for (int i = 0; i < n_pro; i++)
     {
          stato[i] = PENSA;
     }

     for (int i = 0; i < n_pro; i++)
     {
          if (sem_init(&forchette[i], 0, 1) != 0)
          {
               perror("Errore nella creazione del semaforo");
               return 1;
          }
          printf("\nsemaforo creato\n");
     }

     if (strcmp(argv[2], "1") == 0)
     {
          ril_stallo = 1;
          puts("Rilevamento stallo abilitato");
     }
     else
     {
          puts("Rilevamento stallo non abilitato");
     }

     if (strcmp(argv[3], "1") == 0)
     {
          ab_stallo = 1;
          puts("Non evita stallo");
     }
     else
     {
          puts("Evita stallo");
     }

     if (strcmp(argv[4], "1") == 0)
     {
          riv_star = 1;
          puts("Rilevamento starvation abilitato");
     }
     else
     {
          puts("Rilevamento starvation non abilitato");
     }

     printf("Il numero dei processi è %d \n", n_pro);

     struct sharedMemory shm;

     shm.n_pro = n_pro;
     shm.deadlock = n_pro;
     shm.shm_size = sizeof(shm);
     sprintf(shm.msg, "APPENA CREATA\n");

     printf("Premere Ctrl+C per eseguire l'handler di SIGINT\n");
     printf("Potete uscire con Ctrl+\\ (provoca l'invio di SIGQUIT)\n");
     int shm_fd = 0;
     int shmStato_fd = 0;
     int shmSem_fd = 0;
     if ((shm_fd = shm_open(NAMESHM, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) == -1)
     {
          printf("errore apertura shm - %d\n", errno);
          return -1;
     }
     
     if ((shmStato_fd = shm_open(NAMESTATO, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) == -1)
     {
          printf("errore apertura shm stato- %d\n", errno);
          return -1;
     }
     if ((shmSem_fd = shm_open(NAMESEMAFORI, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) == -1)
     {
          printf("errore apertura shm semafori- %d\n", errno);
          return -1;
     } else puts("semafori caricati nella shm");
    
     
     if (ftruncate(shmStato_fd, sizeof(stato)) == -1)
     {
          printf("errore ftruncate - %d", errno);
          exit(1);
     }
     if (ftruncate(shmSem_fd, sizeof(sem_t)*n_pro) == -1)
     {
          printf("errore ftruncate - %d", errno);
          exit(1);
     }
     
     int *shmStato_ptr = (int*)mmap(NULL, sizeof(stato), PROT_READ | PROT_WRITE, MAP_SHARED, shmStato_fd, 0);
     sem_t *shmSem_ptr = (sem_t*)mmap(NULL, sizeof(stato), PROT_READ | PROT_WRITE, MAP_SHARED, shmStato_fd, 0);
     memcpy(shmStato_ptr, stato, sizeof(stato));
     memcpy(shmSem_ptr, forchette, sizeof(forchette));
     if (shmStato_ptr == MAP_FAILED)
     {
          perror("mmap");
          exit(1);
     }
     if (shmSem_ptr == MAP_FAILED)
     {
          perror("mmap");
          exit(1);
     }
     

     if (ftruncate(shm_fd, sizeof(shm)) == -1)
     {
          printf("errore ftruncate - %d", errno);
          return -1;
     }
     void *shm_ptr = mmap(NULL, sizeof(shm), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
     if (shm_ptr == MAP_FAILED)
     {
          perror("mmap");
          exit(1);
     }
     memcpy(shm_ptr, &shm, sizeof(shm));
     
     
     pid_t pid = 0;
     // fork filosofi
     for (int i = 0; i < n_pro; i++)
     {
          
          if ((pid = fork()) == -1)
          {
               fprintf(stderr, "Parent: Errore nel fork - errno: %d \n", errno);
               return -1;
          }
          else if (pid == 0)
          {
               // child
               if(ab_stallo == 0){
                    filosofo(i, sizeof(shm));
               } else {
                    filosofoStallo(i, sizeof(shm));
               }
          }
     }

     signal(SIGINT, SigIntHandler);
     
     printf("parent: pid %d\n", getpid());
     while(deadlock>=0){
     for(int i=0; i<n_pro; i++){
          if(shmStato_ptr[i]==AFFAMATO) deadlock--;
     }
     }

     for (int i = 0; i < n_pro; i++)
     {
          int status;
          wait(&status); // Aspetta che un processo figlio termini
          printf("Processo figlio terminato\n");
          sem_destroy(&forchette[i]);
          munmap(shm_ptr, sizeof(shm));
          munmap(shmSem_ptr, sizeof(shm));
          munmap(shmStato_ptr, sizeof(stato));
          shm_unlink(NAMESHM);
          shm_unlink(NAMESEMAFORI);
          shm_unlink(NAMESTATO);
     }

     return 0;
}
