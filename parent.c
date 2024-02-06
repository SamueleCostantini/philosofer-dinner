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

// Struct per la shared memory contenente variabili importanti per il controllore

struct sharedMemory
{
     pid_t pidParent;      // PID del processo principale (parent)
     int n_pro;            // Numero di filosofi
     int deadlock;         // Indicatore di deadlock
     char msg[256];        // Messaggio per la comunicazione
     int shm_size;         // Dimensione della shared memory
     bool ril_star;        // Flag per il rilevamento di starvation
     bool ril_stallo;      // Flag per il rilevamento di stallo
     pid_t pidControllore; // PID del processo di controllo
};

// signal handler che gestisce il segnale di starvation dal controllore

void SigUsr1Handler(int iSignalCode)
{

     struct sharedMemory *shm_ptr;
     int shm_fd = 0, shmStato_fd = 0, shmSem_fd = 0;
     if ((shm_fd = shm_open(NAMESHM, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) == -1)
     {
          printf("errore apertura shm \n");
          exit(-1);
     }
     if ((shmStato_fd = shm_open(NAMESHM, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) == -1)
     {
          printf("errore apertura shm \n");
          exit(-1);
     }
     if ((shmSem_fd = shm_open(NAMESHM, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) == -1)
     {
          printf("errore apertura shm \n");
          exit(-1);
     }
     shm_ptr = (struct sharedMemory *)mmap(NULL, sizeof(struct sharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
     printf("Ricevuto signal da controllore %d\n", iSignalCode);
     printf("Uno o piu filosofi sono morti di fame!\n");
     printf("Consulta il file dei log per vedere cosa e' successo durante l'esecuzione.\n");
     kill(shm_ptr->pidControllore, SIGTERM); //invio segnale di termine al controllore cosi che si possa fermare e stampare i log
     sprintf(shm_ptr->msg, STOP);  
     munmap(shm_ptr, sizeof(*shm_ptr));
     kill(-getpgrp(), SIGINT);
}

// signal handler che gestisce il segnale di deadlock dal controllore

void SigUsr2Handler(int iSignalCode)
{
      struct sharedMemory *shm_ptr;
     int shm_fd = 0, shmStato_fd = 0, shmSem_fd = 0;
     if ((shm_fd = shm_open(NAMESHM, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) == -1)
     {
          printf("errore apertura shm \n");
          exit(-1);
     }
     if ((shmStato_fd = shm_open(NAMESHM, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) == -1)
     {
          printf("errore apertura shm \n");
          exit(-1);
     }
     if ((shmSem_fd = shm_open(NAMESHM, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) == -1)
     {
          printf("errore apertura shm \n");
          exit(-1);
     }
     shm_ptr = (struct sharedMemory *)mmap(NULL, sizeof(struct sharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
     printf("Ricevuto signal da controllore %d\n", iSignalCode);
     printf("Rilevato uno stallo! Ogni filosofo ha una sola forchetta\n");
     printf("Consulta il file dei log per vedere cosa e' successo durante l'esecuzione.\n");
     kill(shm_ptr->pidControllore, SIGTERM); //invio segnale di termine al controllore cosi che si possa fermare e stampare i log
     sprintf(shm_ptr->msg, STOP);  
     
     munmap(shm_ptr, sizeof(*shm_ptr));
     kill(-getpgrp(), SIGINT);// Invia un segnale di interruzione a tutti i processi figli
     exit(3);
}

// handler per gestire il ctrl+c del parent

void SigIntHandler(int iSignalCode)
{
     printf("Parent (%d) Ricevuto signal %d\n", getpid(), iSignalCode);

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
     
     
}

//signal handler per il sigint dei filosofi

void SigIntHandler1(int iSignalCode)
{
     printf("Filosofo (%d) riceve signal interrupt, pulisco e chiudo - %d\n", getpid(), iSignalCode);
     exit(3);
}

// processo filosofo classico

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
     }
     sem_t *forchette;
     forchette = (sem_t *)mmap(NULL, sizeof(sem_t) * shm_ptr->n_pro, PROT_READ | PROT_WRITE, MAP_SHARED, shmSem_fd, 0);
     if (forchette == MAP_FAILED)
     {
          perror("mmap");
          exit(1);
     }

     int *shmStato_ptr = (int *)mmap(NULL, sizeof(int) * shm_ptr->n_pro, PROT_READ | PROT_WRITE, MAP_SHARED, shmStato_fd, 0);
     if (shmStato_ptr == MAP_FAILED)
     {
          perror("mmap");
          exit(1);
     }

     const int n_pro = shm_ptr->n_pro;
     printf("%d. filosofo: pid %d\n", i + 1, getpid());
     printf("valori shm:\n n_pro: %d\n msg: %s\n", shm_ptr->n_pro, shm_ptr->msg);

     // random number 1 to 10
     srand(time(NULL) + clock());
     int casualNumber = rand() % 10;
     while (strcmp(shm_ptr->msg, STOP) != 1)
     {
          srand(time(NULL) + clock());
          casualNumber = 1 + rand() % 11;
          int j = i + 1;
          if (i == n_pro - 1)
               j = 0;

          strcpy(shm_ptr->msg, "FILOSOFI MANGIANO\n");
          sleep(rand() % 10);
          sem_wait(&forchette[i]);
          printf("%d prende forchetta %d - destra\n", i, i);

          shmStato_ptr[i] = AFFAMATO;

          sem_wait(&forchette[j]);
          printf("%d prende forchetta %d - sinistra\n", i, j);
          shmStato_ptr[i] = MANGIA;

          printf("%d Mangia - %dsec\n", i, casualNumber);
          sleep(casualNumber);

          sem_post(&forchette[i]);

          sem_post(&(forchette[j]));
          printf("%d lascia forchetta destra e sinistra\n", i);
     }
     // shm_unlink(NAMESHM);
     strcpy(shm_ptr->msg, "FILOSOFI FINISCONO DI MANGIARE\n");
     return;
     
}
// processo filosofo che evita lo stallo
void filosofo_no_Stallo(int i, int sizeShm)
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
     }
     sem_t *forchette;
     forchette = (sem_t *)mmap(NULL, sizeof(sem_t) * shm_ptr->n_pro, PROT_READ | PROT_WRITE, MAP_SHARED, shmSem_fd, 0);
     if (forchette == MAP_FAILED)
     {
          perror("mmap");
          exit(1);
     }

     int *shmStato_ptr = (int *)mmap(NULL, sizeof(int) * shm_ptr->n_pro, PROT_READ | PROT_WRITE, MAP_SHARED, shmStato_fd, 0);
     if (shmStato_ptr == MAP_FAILED)
     {
          perror("mmap");
          exit(1);
     }

     const int n_pro = shm_ptr->n_pro;
     printf("%d. filosofo: pid %d\n", i + 1, getpid());
     printf("valori shm:\n n_pro: %d\n msg: %s\n", shm_ptr->n_pro, shm_ptr->msg);

     // random number 1 to 10
     srand(time(NULL) + clock());
     int casualNumber = rand() % 10;
     while (strcmp(shm_ptr->msg, STOP) != 1)
     {
          srand(time(NULL) + clock());
          casualNumber = 1 + rand() % 11;
          int j = i + 1;
          if (i == n_pro - 1)
               j = 0;

          strcpy(shm_ptr->msg, "FILOSOFI MANGIANO\n");
          sleep(rand() % 10);

          printf("%d prende forchetta %d - destra\n", i, i);
          while (sem_trywait(&forchette[i]) != 0 && sem_trywait(&forchette[j]) != 0)
          {
               shmStato_ptr[i] = AFFAMATO;
          }
          

          printf("%d prende forchetta %d - sinistra\n", i, j);
          shmStato_ptr[i] = MANGIA;

          printf("%d Mangia - %dsec\n", i, casualNumber);
          sleep(casualNumber);

          sem_post(&forchette[i]);

          sem_post(&(forchette[j]));
          printf("%d lascia forchetta destra e sinistra\n", i);
     }
     
     strcpy(shm_ptr->msg, "FILOSOFI FINISCONO DI MANGIARE\n");
     munmap(forchette, sizeof(sem_t) * n_pro);
     munmap(shm_ptr, sizeof(shm_ptr));
     munmap(shmStato_ptr, sizeof(int) * n_pro);
    
}
int main(int argc, char *argv[])
{

     // controllo se sono stati inseriti gli argomenti
     if (argc == 1)
     {
          printf("Devi inserire gli argomenti per eseguire il programma.\n");
          return -1;
     }

     // variabili

     const int n_pro = atoi(argv[1]);                     //numero dei filosofi  
     bool ril_stallo = 0, evita_stallo = 0, ril_star = 0; //flag
     sem_t forchette[n_pro];                              //forchette che useranno i filosofi
     int stato[n_pro];                                    //azione attuale dei filosofi: AFFAMATO, MANGIA                            
     int shm_fd = 0;                                      //file descriptor della shm principale
     int shmStato_fd = 0;                                 //file descriptor della shm dello stato dei filosofi
     int shmSem_fd = 0;                                   //file descriptor della shm dei semafori
     
     // -- //

     srand(time(NULL));

     // inizializzazione vettore stato

     for (int i = 0; i < n_pro; i++)
     {
          stato[i] = AFFAMATO;
     }

     // inizializzazione semafori

     for (int i = 0; i < n_pro; i++)
     {
          if (sem_init(&forchette[i], 0, 1) != 0)
          {
               perror("Errore nella creazione del semaforo");
               return 1;
          }
          printf("Semaforo creato\n");
     }

     // verifica flag attivi

     if (strcmp(argv[2], "1") == 0)
     {
          ril_stallo = 1;
          puts("Rilevamento stallo abilitato");
     }
     else
     {
          ril_stallo = 0;
          puts("Rilevamento stallo non abilitato");
     }

     if (strcmp(argv[3], "1") == 0)
     {
          evita_stallo = 1;
          puts("Evita stallo");
     }
     else
     {
          evita_stallo = 0;
          puts("Non evita stallo");
     }

     if (strcmp(argv[4], "1") == 0)
     {
          ril_star = 1;
          puts("Rilevamento starvation abilitato");
     }
     else
     {
          ril_star = 0;
          puts("Rilevamento starvation non abilitato");
     }

     printf("Il numero dei processi è %d \n", n_pro);

     // inizializzazione struct shared memory
     // variabili utili

     struct sharedMemory shm;
     shm.ril_stallo = ril_stallo;
     shm.ril_star = ril_star;
     shm.n_pro = n_pro;
     shm.deadlock = n_pro;
     shm.pidParent = getpid();
     shm.shm_size = sizeof(shm);
     pid_t pid = 0;

     // -- //

     sprintf(shm.msg, "APPENA CREATA\n");

     printf("Premere Ctrl+C per eseguire l'handler di SIGINT\n");
     printf("Potete uscire con Ctrl+\\ (provoca l'invio di SIGQUIT)\n");

     // apertura shared memory principale

     if ((shm_fd = shm_open(NAMESHM, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) == -1)
     {
          printf("errore apertura shm - %d\n", errno);
          return -1;
     }

     // apertura shared memory vettore di stato che conterra lo stato di ogni filosofo: AFFAMATO o MANGIA

     if ((shmStato_fd = shm_open(NAMESTATO, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) == -1)
     {
          printf("errore apertura shm stato- %d\n", errno);
          return -1;
     }

     // apertura shared memory vettore dei semafori

     if ((shmSem_fd = shm_open(NAMESEMAFORI, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) == -1)
     {
          printf("errore apertura shm semafori- %d\n", errno);
          return -1;
     }
     else
          puts("semafori caricati nella shm");

     // ridimensionamento memoria

     if (ftruncate(shmStato_fd, sizeof(stato)) == -1)
     {
          printf("errore ftruncate - %d", errno);
          exit(1);
     }
     if (ftruncate(shmSem_fd, sizeof(sem_t) * n_pro) == -1)
     {
          printf("errore ftruncate - %d", errno);
          exit(1);
     }
     if (ftruncate(shm_fd, sizeof(shm)) == -1)
     {
          printf("errore ftruncate - %d", errno);
          return -1;
     }

     // mapping delle shared memory

     int *shmStato_ptr = (int *)mmap(NULL, sizeof(stato), PROT_READ | PROT_WRITE, MAP_SHARED, shmStato_fd, 0);
     sem_t *shmSem_ptr = (sem_t *)mmap(NULL, sizeof(sem_t) * n_pro, PROT_READ | PROT_WRITE, MAP_SHARED, shmSem_fd, 0);
     struct sharedMemory *shm_ptr = (struct sharedMemory *)mmap(NULL, sizeof(shm), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

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
     if (shm_ptr == MAP_FAILED)
     {
          perror("mmap");
          exit(1);
     }


     // caricamento dati nelle shared memory

     memcpy(shm_ptr, &shm, sizeof(shm));
     memcpy(shmStato_ptr, stato, sizeof(stato));
     memcpy(shmSem_ptr, forchette, sizeof(sem_t) * n_pro);

     // signal handler del parent

     if (ril_star)
          signal(SIGUSR1, SigUsr1Handler);

     if (ril_stallo)
          signal(SIGUSR2, SigUsr2Handler);

     signal(SIGINT, SigIntHandler);

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
               if (evita_stallo == 0)
               {
                    filosofo(i, sizeof(shm)); // filosofo con stallo
                  
               }
               else
               {
                    filosofo_no_Stallo(i, sizeof(shm)); // filosofo senza stallo
                    
               }
          }
     }

     printf("parent: pid %d\n", getpid());

     for (int i = 0; i < n_pro; i++)
     {
          // attesa chiusura filosofi

          int status;
          wait(&status); // Aspetta che un processo figlio termini
          printf("Processo figlio terminato\n");

          // pulizia della memoria

          sem_destroy(&forchette[i]);
     }
     printf("Controllore pid (%d) interrotto\n", shm_ptr->pidControllore);
     printf("Controlla il file di log per vedere quello che e' successo.\n"); 
     
     /*
          il controllore ha stampato il file di log 
          dove c'è il resoconto dell'esecuzione del programma

     */

     //pulizia
     kill(shm_ptr->pidControllore, SIGTERM); //segnale ti term inviato al controllore cosi che si possa fermare
     munmap(shm_ptr, sizeof(shm));
     munmap(shmSem_ptr, sizeof(shm));
     munmap(shmStato_ptr, sizeof(stato));
     shm_unlink(NAMESHM);
     shm_unlink(NAMESEMAFORI);
     shm_unlink(NAMESTATO);

     return 0;
}
