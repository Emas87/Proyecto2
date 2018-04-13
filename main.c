#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/shm.h>
#include "Semaforo.h"
#include "decoder.h"

#define SHSIZE 100

int shmmap(key_t key,char **shm,int shsize){
   int shmid = shmget(key,shsize,IPC_CREAT | 0666);
   if (shmid < 0){
      perror("shmget");
      exit(1);
   }
   *shm = shmat(shmid,NULL,0);
   if(shm == (void *)-1) {
      perror("shmmat");
      exit(1);
   }
   return shmid;
}
void shmunmap(int shmid,char *shm){
   struct shmid_ds *shmid_buf = NULL;
   int status = shmdt(shm);
   if (status < 0){
      perror("shmdt");
      exit(1);
   }
   status = shmctl(shmid,IPC_RMID,shmid_buf);
   if (status < 0){
      perror("shmclt");
      exit(1);
   }
}


int main(int argc,char *argv[]){
   int shmid_buf,shmid_bandera,shmid_cont_prod,shmid_cont_cons;
   key_t key,key_semaforo,key_bandera,key_cont_prod,key_cont_cons;
   char *shm = NULL,*shm_cont_prod = NULL,*shm_cont_cons = NULL,*shm_bandera = NULL;
   char *s;
   int semid = 0;

   s = "buffer";
   key = decoder(s); //key del buffer
   s = "bandera";   
   key_bandera=decoder(s); //key de la bandera
   s = "contador1";   
   key_cont_prod=decoder(s); //key del contador de productores
   s = "contador2";   
   key_cont_cons=decoder(s); //key del contador de consumidores
   s = "semaforo";   
   key_semaforo = decoder(s); //key del semaforo
   
   //Semaforo
   semid = Semaforo(key_semaforo);
   printf("semid: %d\n",semid);

   shmid_buf = shmmap(key,&shm,SHSIZE);
   shmid_bandera = shmmap(key_bandera,&shm_bandera,SHSIZE);
   shmid_cont_prod = shmmap(key_cont_prod,&shm_cont_prod,SHSIZE);
   shmid_cont_cons = shmmap(key_cont_cons,&shm_cont_cons,SHSIZE);
   getSemaphore(key_semaforo);

   memcpy(shm,"hola",4);
   s = shm;
   s =s + 4;
   *s = 0;
   Wait(semid);

   while(*shm != '*'){
      sleep(1);
   }
   Signal(semid);
   
   shmunmap(shmid_buf,shm);
   shmunmap(shmid_bandera,shm_bandera);
   shmunmap(shmid_cont_prod,shm_cont_prod);
   shmunmap(shmid_cont_cons,shm_cont_cons);
   RemSem(semid);


   return 0;
}

/*int main() {
    Semaforo s;
    int j;

    if ( fork() ) {
       	for (int i = 0; i < 10; i++ ){
           printf( "Esperando para activar el semaforo %d \n", i );
	}
       	scanf( "%d", &j );
	exit(-1);
       	s.Signal();
    }
    else {
       printf( "Esperando que el semaforo se active  ...\n" );
       s.Wait();
       printf( "Semaforo activado\n" );
    }
*/
