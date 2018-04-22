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
   semid = Semaforo(key_semaforo,sem_size);
   printf("semid: %d\n",semid);

   shmid_buf = shmmap(key,&shm,SHSIZE+(MSJSIZE)*5); //modificar 5 por variable de entrada
   shmid_bandera = shmmap(key_bandera,&shm_bandera,1); //char
   shmid_cont_prod = shmmap(key_cont_prod,(char **)&shm_cont_prod,8);//long int
   shmid_cont_cons = shmmap(key_cont_cons,(char **)&shm_cont_cons,8);//long int
   //getSemaphore(key_semaforo);
/*
   memcpy(shm_bandera,"0",1);
   *shm_cont_prod = 0;
   *shm_cont_cons = 0;
   int init = 0;
   memcpy(shm,&init,sizeof(long int));
   s = &shm[16];   
   init = 5;   
   memcpy(s,&init,sizeof(long int));

   s = shm;
   //s =s + 4;
   *s = 0;
*/
   char bandera = '0';
   while(bandera != '1'){
      sleep(1);
      Wait(semid,1); //protocolo de entrada
      bandera = *shm_bandera;
      Signal(semid,1); //protocolo de salida
   }
   s = &shm[SHSIZE];
   //long int *id = (long int*)&shm[2];
   time_t *t = (time_t*)&shm[SHSIZE+8];
   int * aleatorio = (int*)&shm[SHSIZE+16];
   printf("id : %ld\n",(long int)*s);
   printf("aleatorio : %d\n",*aleatorio);

   shmunmap(shmid_buf,shm);
   shmunmap(shmid_bandera,shm_bandera);
   shmunmap(shmid_cont_prod,(char *)shm_cont_prod);
   shmunmap(shmid_cont_cons,(char *)shm_cont_cons);
   RemSem(semid);

   
   return 0;
}
