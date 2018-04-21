#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>
#include "Semaforo.h"
#include "decoder.h"


#define SHSIZE 8
#define MSJSIZE 100

int shmmap(key_t key,char **shm,int shsize){
   int shmid = shmget(key,shsize,0666);
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
   //long int *n;
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

   shmid_buf = shmmap(key,&shm,SHSIZE+5);
   shmid_bandera = shmmap(key_bandera,&shm_bandera,1);
   shmid_cont_prod = shmmap(key_cont_prod,&shm_cont_prod,4);
   shmid_cont_cons = shmmap(key_cont_cons,&shm_cont_cons,4);
   semid = getSemaphore(key_semaforo);

   // Se obtiene el id del productor leyendo la shm de contador de productor
   // se usa el semaforo antes de leerlo, se asigna el id, se escribe y se hace Signal(semid);
   long int id = 0;
   Wait(semid,2); //protocolo de entrada
   id = *shm_cont_prod; id++;
   *shm_cont_prod = id;
   Signal(semid,2); //protocolo de salida
   
   
   // Se crea un loop hasta que se active la bandera
   char bandera = '0';
   while(bandera != '1'){
      // En el loop se genera el tiempo de espera aleatorio, se espera y
      // se pide el semaforo con wait    Wait(semid);
      Wait(semid,0); //protocolo de entrada
      // se lee la posicion del buffer(indice), leyendo la primera posicion del buffer
      long int indice = (long int)shm[0];
      long int nbuffer = (long int)shm[1];
      s = shm[8]; // a partir del byte 8 esta el array para los mensajes
      s + MSJSIZE*indice;//apuntar la posicion para escribir mensaje

      // escribe el mensaje(id del prod,fecha y hora,llave aleatoria entre 0 y 4)
      memcpy(s,"0",1);
      memcpy(s,"-",1);
      memcpy(s,"0",1);
      memcpy(s,"-",1);
      memcpy(s,"0",1);

      Signal(semid,0); //protocolo de salida

      // se imprime en consola describiendo la accion realizada,incluyendo el indice y la cantidad de prod/consum
      // Signal(semid,snum);
      //Verificar bandera
      Wait(semid,3); //protocolo de entrada
      //bandera = *shm_bandera;
      bandera = '1';
      *shm_bandera = bandera;
      Signal(semid,3); //protocolo de salida
   }
   Wait(semid,2); //protocolo de entrada
   id = *shm_cont_prod; id--;
   *shm_cont_prod = id;
   Signal(semid,2); //protocolo de salida

   // Luego de activarse la bandera se debe decrementar el contador de de prod
   // Salir

   //for(s = shm; *s != 0;s++){
   //   printf("%c",*s);
   //}
   printf("\n");
   *shm_bandera = '1';

   /*shmunmap(shmid_buf,shm);
   shmunmap(shmid_bandera,shm_bandera);
   shmunmap(shmid_cont_prod,shm_cont_prod);
   shmunmap(shmid_cont_cons,shm_cont_cons);*/
   return 0;
}
