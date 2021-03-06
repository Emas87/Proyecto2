#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/shm.h>
#include "Semaforo.h"
#include "decoder.h"
#include "run.h"


#define SHSIZE 24 // 8 bytes para index de prod, 8 bytes para index de consumidores, 8 bytes para tamano del buffer y 8 bytes para llevar el ultimo ID de consumidores
#define SEMSIZE 4 // Solo 4 semaforos se ocupan, para el buffer, para la bandera, y para los dos contadores
// 0 -> Buffer
// 1 -> Bandera
// 2 -> Contador Productores
// 3 -> Contador Consumidores

int shmmap(key_t key,char **shm,int shsize){
   int shmid = shmget(key,shsize,IPC_CREAT| IPC_EXCL | 0666);
   if (shmid < 0){
      printf("No se pudo crear la memoria compartida con ese key: %ld\n",(long int)key);
      exit(1);
   }
   *shm = shmat(shmid,NULL,0);
   if(shm == (void *)-1) {
      printf("No se pudo mapear la memoria compartida con ese key: %ld\n",(long int)key);
      exit(1);
   }
   return shmid;
}
void shmunmap(int shmid,char *shm){
   struct shmid_ds *shmid_buf = NULL;
   int status = shmdt(shm);
   if (status < 0){
      perror("No se pudo remover esa memoria compartida");
      exit(1);
   }
   status = shmctl(shmid,IPC_RMID,shmid_buf);
   if (status < 0){
      perror("No se pudo remover esa memoria compartida");
      exit(1);
   }
}


int main(int argc,char *argv[]){
   int modo= 0;
   char buff[100];
   char *buffer; buffer = buff;
   long int tam;
   long int* tamano;tamano = &tam;
   double tiem;
   double* tiempo;tiempo = &tiem;

   printf("********************************************\nEjecucion de Creador\n********************************************\n");

   parser(modo, &buffer, tamano, tiempo, argc, argv);

   int MSJSIZE = sizeof(long int) + sizeof(time_t) + sizeof(int); // Se define asi para que pueda ser portable

   int shmid_buf,shmid_bandera,shmid_cont_prod,shmid_cont_cons;
   key_t key,key_semaforo,key_bandera,key_cont_prod,key_cont_cons;
   char *shm = NULL, *shm_bandera = NULL;
   long int *shm_cont_prod = NULL,*shm_cont_cons = NULL;
   char *s;
   int semid = 0,sem_size = SEMSIZE;

   s = buffer;
   key = decoder(s,""); // Key del buffer
   key_bandera=decoder(s,"band"); // Key de la bandera
   key_cont_prod=decoder(s,"prod"); // Key del contador de productores
   key_cont_cons=decoder(s,"cons"); // Key del contador de consumidores
   key_semaforo = decoder(s,"sema"); // Key del semaforo
   
   // Semaforo
   semid = Semaforo(key_semaforo,sem_size);

   shmid_buf = shmmap(key,&shm,SHSIZE+(MSJSIZE) * *tamano); 
   shmid_bandera = shmmap(key_bandera,&shm_bandera,1);
   shmid_cont_prod = shmmap(key_cont_prod,(char **)&shm_cont_prod,8);
   shmid_cont_cons = shmmap(key_cont_cons,(char **)&shm_cont_cons,8);

   memcpy(shm_bandera,"0",1);
   long int init = 0;
   memcpy(shm_cont_prod,&init,sizeof(long int)); // Contador de productor = 0
   memcpy(shm_cont_cons,&init,sizeof(long int)); // Contador de consumidor = 0
   memcpy(shm,&init,sizeof(long int)); // Indice de productor = 0
   s = &shm[8];
   memcpy(s,&init,sizeof(long int)); // Indice de consumidor = 0
   s = &shm[16];   
   init = *tamano;   
   memcpy(s,&init,sizeof(long int)); // Escribir tamano del buffer
   s = &shm[24];
   init = 0;   
   memcpy(s,&init,sizeof(long int)); // ID de consumidor = 0

   printf("********************************************\n");

   return 0;
}
