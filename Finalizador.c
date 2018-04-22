#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/shm.h>
#include "Semaforo.h"
#include "decoder.h"

#define SHSIZE 24 //8 bytes para index de prod,8 bytes para index de consumidores, 8 bytes para tamano del buffer
#define SEMSIZE 4 //solo 4 semaforos se ocupan, para el buffer, para la banndera, y para los dos contadores
//0 buffer
//1 bandera
//2 contador Productores
//3 contador Consumidores


int shmmap(key_t key,char **shm,int shsize){
   int shmid = shmget(key,shsize, 0666);
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
   int MSJSIZE = sizeof(long int) + sizeof(time_t) + sizeof(int);//se define asi para que pueda ser portable

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
   semid = getSemaphore(key_semaforo);
   printf("semid: %d\n",semid);

   shmid_buf = shmmap(key,&shm,SHSIZE+(MSJSIZE)*5); //modificar 5 por variable de entrada
   shmid_bandera = shmmap(key_bandera,&shm_bandera,1); //char
   shmid_cont_prod = shmmap(key_cont_prod,(char **)&shm_cont_prod,8);//long int
   shmid_cont_cons = shmmap(key_cont_cons,(char **)&shm_cont_cons,8);//long int

   //activar bandera para productores
   char bandera = '1';   
   Wait(semid,1); //protocolo de entrada
   *shm_bandera = bandera;
   Signal(semid,1); //protocolo de salida


   //Escribir en todas las posiciones de memoria TODO: se ocupa asegurar que ningun productor vaya a sobreescribir sobre esto
   Wait(semid,0); //protocolo de entrada
   s = &shm[0];
   long int *indice = (long int*)s;
   long int *tamano = (long int*)&shm[8];
   printf("indice : %ld\n",*indice);
   printf("tamano : %ld\n",*tamano);
   s = &shm[SHSIZE]; 
   for(long int i = 0;i<*tamano;i++){
      // escribe el mensaje(id del prod,fecha y hora,llave aleatoria entre 0 y 4)
      //long int,time_t,int
      memset(s, 0, sizeof(long int));
      s+= sizeof(long int);
      memset(s,0,sizeof(time_t));
      s+= sizeof(time_t);
      memset(s,0,sizeof(int));
      s+= sizeof(int);
   }
   Signal(semid,0); //protocolo de salida      

   shmunmap(shmid_buf,shm);
   shmunmap(shmid_bandera,shm_bandera);
   shmunmap(shmid_cont_prod,(char *)shm_cont_prod);
   shmunmap(shmid_cont_cons,(char *)shm_cont_cons);
   RemSem(semid);

   
   return 0;
}
