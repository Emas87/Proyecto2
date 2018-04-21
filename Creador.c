#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/shm.h>
#include "Semaforo.h"
#include "decoder.h"

#define SHSIZE 8 //4 bytes para index,4 bytes para tamano del buffer
#define MSJSIZE 100
#define SEMSIZE 4 //Creo que son solo 4 semaforos que se ocupan, para el buffer, para la banddera, y para los dos contadores
//0 buffer
//1 bandera
//2 contador Productores
//3 contador Consumidores

int shmmap(key_t key,char **shm,int shsize){
   int shmid = shmget(key,shsize,IPC_CREAT| IPC_EXCL | 0666);
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
   char *shm = NULL, *shm_bandera = NULL;
   long int *shm_cont_prod = NULL,*shm_cont_cons = NULL;
   char *s;
   int semid = 0,sem_size = SEMSIZE;

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
   semid = Semaforo(key_semaforo,sem_size);
   printf("semid: %d\n",semid);

   shmid_buf = shmmap(key,&shm,SHSIZE+5); //modificar 5 por variable de entrada
   shmid_bandera = shmmap(key_bandera,&shm_bandera,1); //char
   shmid_cont_prod = shmmap(key_cont_prod,&shm_cont_prod,4);//long int
   shmid_cont_cons = shmmap(key_cont_cons,&shm_cont_cons,4);//long int
   getSemaphore(key_semaforo);

   memcpy(shm_bandera,"0",1);
   *shm_cont_prod = 0;
   *shm_cont_cons = 0;
   s = shm;
   //s =s + 4;
   *s = 0;
   Wait(semid,0);
   char bandera = '0';
   while(bandera != '1'){
      sleep(1);
      Wait(semid,3); //protocolo de entrada
      bandera = *shm_bandera;
      Signal(semid,3); //protocolo de salida
   }
   Signal(semid,0);
   
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
