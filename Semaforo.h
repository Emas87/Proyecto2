#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdlib.h>
#include <errno.h>

int Semaforo(key_t key ){
   int ValorInicial = 0;
   int id;		// Identificador del semaforo   
   struct sembuf sbuf;
   id = semget(key,1, 0775 | IPC_CREAT | IPC_EXCL );
   if(id< 0){
      perror("Can\'t get id due to");
      exit(-1);
   } else if (id != -1){
      sbuf.sem_num = 0;
      sbuf.sem_op = ValorInicial;  /* This is the number of runs without queuing. */
      sbuf.sem_flg = 0;
      if (semop(id, &sbuf, 1) == -1) {
         perror("error: semop"); exit(1);
      }
   } else if (errno == EEXIST) {
      id = semget(key, 0, 0);
      if (id == -1) {
         perror("error 1: semget"); exit(1);
      }
   } else {
      printf("id: %d\n",id);
      perror("error 2: semget"); exit(1);
   }
   return id;
}

void Wait(int id) {
   struct sembuf sbuf;
	sbuf.sem_num = 0;
	sbuf.sem_op = -1;  
	sbuf.sem_flg = 0;
	if (semop(id, &sbuf, 1) == -1) {
      printf("Wait\n");
		perror("IPC error: semop"); exit(1);
	}
}

void Signal(int id) {
   struct sembuf sbuf;
	sbuf.sem_num = 0;
	sbuf.sem_op = 1; 
	sbuf.sem_flg = 0;
	if (semop(id, &sbuf, 1) == -1) {
      printf("Signal\n");      
	   perror("IPC error: semop"); exit(1);
	}
}

void RemSem(int semid){
   int status = 0;
   status = semctl(semid,0,IPC_RMID);
   if (status < 0) {
	   perror("semctl error: IPC_RMID"); exit(1);
	}
}
