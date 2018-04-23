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
      if (errno == EEXIST) {
	      id = semget(key, 0, 0);
	      if (id == -1) {
	         perror("No se pudo creaer el semaforo, ya existe"); exit(1);
	      }
	   } else {
	      printf("id: %d\n",id);
	      perror("No se pudo inicializar el semaforo: semget"); exit(1);
	   }

      perror("No se pudo creaer el semaforo, ya existe");
      exit(-1);
   } else if (id != -1){
      //inicializar semaforos
      int i;
      for(i =0;i<sem_size;i++){
         sbuf.sem_num = i;
         sbuf.sem_op = ValorInicial;  /* This is the number of runs without queuing. */
         sbuf.sem_flg = 0;
         if (semop(id, &sbuf, 1) == -1) {
            perror("No se pudo inicializar el semaforo: semop"); exit(1);
         }
      }
   } else   return id;
}

void Wait(int id,int snum) {//P
   struct sembuf sbuf;
	sbuf.sem_num = snum;
	sbuf.sem_op = -1;  
	sbuf.sem_flg = 0;
	if (semop(id, &sbuf, 1) == -1) {
      printf("Wait\n");
		perror("No se pudo ejecutar semop para hacer Wait"); exit(1);
	}
}

void Signal(int id,int snum) {//V
   struct sembuf sbuf;
	sbuf.sem_num = snum;
	sbuf.sem_op = 1; 
	sbuf.sem_flg = 0;
	if (semop(id, &sbuf, 1) == -1) {
      printf("Signal\n");      
	   perror("No se pudo ejecutar semop para hacer Signal"); exit(1);
	}
}

void RemSem(int semid){
   int status = 0;
   status = semctl(semid,0,IPC_RMID);
   if (status < 0) {
	   printf("no se pudo remover el semaforo con id %d",semid); exit(1);
	}
}

int getSemaphore(key_t semkey) {
	int semid = 0;
	
	// Get semaphore ID associated with this key. 
	if ((semid = semget(semkey, 0, 0)) == -1) {
      perror("No existe semaforo asociado a ese key"); exit(1);
	}
	
	printf("semid: %d\n", semid);
	
	return semid;
}
