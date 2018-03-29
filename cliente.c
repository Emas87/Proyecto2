#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>
#include "Semaforo.h"
#include "decoder.h"


#define SHSIZE 100

int main(int argc,char *argv[]){
   int shmid;
   key_t key,key_semaforo,key_bandera,key_cont_prod,key_cont_cons;
   char *shm;
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

   shmid = shmget(key,SHSIZE,0666);
   if (shmid < 0){
      perror("shmget");
      exit(1);
   }
   shm = shmat(shmid,NULL,0);
   if (shm == (char *) -1){
      perror("shmmat");
      exit(1);
   }

   for(s = shm; *s != 0;s++){
      printf("%c",*s);
   }
   printf("\n");
   *shm = '*';
   shmdt(shm);

   return 0;
}
