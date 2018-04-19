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

   // Leer datos de la linea de comandos
   //
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

   shmid_buf = shmmap(key,&shm,SHSIZE);
   shmid_bandera = shmmap(key_bandera,&shm_bandera,SHSIZE);
   shmid_cont_prod = shmmap(key_cont_prod,&shm_cont_prod,SHSIZE);
   shmid_cont_cons = shmmap(key_cont_cons,&shm_cont_cons,SHSIZE);

   // Se obtiene el id del productor leyendo la shm de contador de productor
   // se usa el semaforo antes de leerlo, se asigna el id, se escribe y se hace Signal(semid);
   // Se crea un loop hasta que se active la bandera
      // En el loop se genera el tiempo de espera aleatorio, se espera y
      // se pide el semaforo con wait    Wait(semid);
      // se lee la posicion del buffer(indice), leyendo la primera posicion del buffer
      // escribe el mensaje(id del prod,fecha y hora,llave aleatoria entre 0 y 4)
      // se imprime en consola describiendo la accion realizada,incluyendo el indice y la cantidad de prod/consum
      // Signal(semid);
   // Luego de activarse la bandera se debe decrementar el contador de de prod
   // Salir

   for(s = shm; *s != 0;s++){
      printf("%c",*s);
   }
   printf("\n");
   *shm = '*';

   /*shmunmap(shmid_buf,shm);
   shmunmap(shmid_bandera,shm_bandera);
   shmunmap(shmid_cont_prod,shm_cont_prod);
   shmunmap(shmid_cont_cons,shm_cont_cons);*/
   return 0;
}
