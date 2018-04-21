#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdlib.h>
#include <errno.h>

int Semaforo(key_t key, int sem_size ){
   int ValorInicial = 1;
   int id;		// Identificador del semaforo   
   struct sembuf sbuf;
   id = semget(key,sem_size, 0775 | IPC_CREAT | IPC_EXCL );
   if(id< 0){
      perror("Can\'t get id due to");
      exit(-1);
   } else if (id != -1){
      //inicializar semaforos
      for(int i =0;i<sem_size;i++){
         sbuf.sem_num = i;
         sbuf.sem_op = ValorInicial;  /* This is the number of runs without queuing. */
         sbuf.sem_flg = 0;
         if (semop(id, &sbuf, 1) == -1) {
            perror("error: semop"); exit(1);
         }
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

void Wait(int id,int snum) {//P
   struct sembuf sbuf;
	sbuf.sem_num = snum;
	sbuf.sem_op = -1;  
	sbuf.sem_flg = 0;
	if (semop(id, &sbuf, 1) == -1) {
      printf("Wait\n");
		perror("IPC error: semop"); exit(1);
	}
}

void Signal(int id,int snum) {//V
   struct sembuf sbuf;
	sbuf.sem_num = snum;
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

int getSemaphore(key_t semkey) {
	int semid = 0;
	
	// Get semaphore ID associated with this key. 
	if ((semid = semget(semkey, 0, 0)) == -1) {
	        perror("IPC error 2: semget"); exit(1);
	}
	
	printf("semid: %d\n", semid);
	
	return semid;
}
