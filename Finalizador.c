#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/shm.h>
#include <time.h>
#include "Semaforo.h"
#include "decoder.h"
#include "run.h"

#define SHSIZE 24 //8 bytes para index de prod,8 bytes para index de consumidores, 8 bytes para tamano del buffer y 8 bytes para llevar el ultimo ID de consumidores
#define SEMSIZE 4 //solo 4 semaforos se ocupan, para el buffer, para la banndera, y para los dos contadores
//0 buffer
//1 bandera
//2 contador Productores
//3 contador Consumidores


int shmmap(key_t key,char **shm,int shsize){
   int shmid = shmget(key,shsize, 0666);
   if (shmid < 0){
      printf("no existe memoria compartida asociada a ese key: %ld\n",(long int)key);
      exit(1);
   }
   *shm = shmat(shmid,NULL,0);
   if(shm == (void *)-1) {
      printf("no existe memoria compartida asociada a ese key: %ld\n",(long int)key);
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
   int modo= 2;
   char buff[100];
   char *buffer; buffer = buff;
   long int tam;
   long int* tamano;tamano = &tam;
   double tiem;
   double* tiempo;tiempo = &tiem;
   parser(modo, &buffer, tamano, tiempo, argc, argv);

   struct timespec antes,despues;   
   int MSJSIZE = sizeof(long int) + sizeof(time_t) + sizeof(int);//se define asi para que pueda ser portable

   int shmid_buf,shmid_bandera,shmid_cont_prod,shmid_cont_cons;
   key_t key,key_semaforo,key_bandera,key_cont_prod,key_cont_cons;
   char *shm = NULL, *shm_bandera = NULL;
   long int *shm_cont_prod = NULL,*shm_cont_cons = NULL;
   char *s;
   int semid = 0,sem_size = SEMSIZE;

   clock_gettime ( CLOCK_REALTIME,  &antes );//Calcular tiempo que dura el finalizador

   s = buffer;
   key = decoder(s,""); //key del buffer
   key_bandera=decoder(s,"band"); //key de la bandera
   key_cont_prod=decoder(s,"prod"); //key del contador de productores
   key_cont_cons=decoder(s,"cons"); //key del contador de consumidores
   key_semaforo = decoder(s,"sema"); //key del semaforo
   
   //Semaforo
   semid = getSemaphore(key_semaforo);

   shmid_buf = shmmap(key,&shm,0);
   shmid_bandera = shmmap(key_bandera,&shm_bandera,1); //char
   shmid_cont_prod = shmmap(key_cont_prod,(char **)&shm_cont_prod,8);//long int
   shmid_cont_cons = shmmap(key_cont_cons,(char **)&shm_cont_cons,8);//long int

   //activar bandera para productores
   char bandera = '1';   
   Wait(semid,1); //protocolo de entrada
      *shm_bandera = bandera;
   Signal(semid,1); //protocolo de salida


   //Escribir en todas las posiciones de memoria 
   Wait(semid,0); //protocolo de entrada
      s = shm;
      long int *indicep = (long int*)s;
      s+=8;
      long int *indicec = (long int*)s;
      s+=8;
      tamano = (long int*)s;
      printf("indicep : %ld\n",*indicep);
      printf("indicec : %ld\n",*indicec);
      printf("tamano : %ld\n",*tamano);
      s = &shm[SHSIZE];
           long int i = 0; 
	   for(i = 0;i<*tamano;i++){
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

   long int contador_Prod = 1;
   long int contador_Cons = 1;
   while(contador_Prod > 0 | contador_Cons > 0){
      Wait(semid,2); //protocolo de entrada
         contador_Prod = *shm_cont_prod;
      Signal(semid,2); //protocolo de salida

      Wait(semid,3); //protocolo de entrada
         contador_Cons = *shm_cont_cons;
      Signal(semid,3); //protocolo de salida
   }


   shmunmap(shmid_buf,shm);
   shmunmap(shmid_bandera,shm_bandera);
   shmunmap(shmid_cont_prod,(char *)shm_cont_prod);
   shmunmap(shmid_cont_cons,(char *)shm_cont_cons);
   RemSem(semid);

   clock_gettime ( CLOCK_REALTIME,  &despues );
   double acumulado_tiempo = (despues.tv_sec - antes.tv_sec) + (despues.tv_nsec - antes.tv_nsec)*1e-9;
   printf("Termino Finalizador\nTiempo de ejecucion: %.10lf\n",acumulado_tiempo);
   
   return 0;
}
