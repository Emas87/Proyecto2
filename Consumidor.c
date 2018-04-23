#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <time.h>
#include "Semaforo.h"
#include "decoder.h"
#include "run.h"
#include "dist.h"

#define SHSIZE 24 // 8 bytes para index de prod, 8 bytes para index de consumidores, 8 bytes para tamano del buffer y 8 bytes para llevar el ultimo ID de consumidores

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
   int modo= 1;
   char buff[100];
   char *buffer; buffer = buff;
   long int tam;
   long int* tamano;tamano = &tam;
   double tiem;
   double* tiempo;tiempo = &tiem;

   printf("********************************************\nEjecucion de Consumidor\n********************************************\n");

   parser(modo, &buffer, tamano, tiempo, argc, argv);

   int shmid_buf,shmid_bandera,shmid_cont_prod,shmid_cont_cons;
   int MSJSIZE = sizeof(long int) + sizeof(time_t) + sizeof(int); // Se define asi para que pueda ser portable
   long int numero_mensajes_enviados=0;
   double acumulado_tiempo_esperados=0.0,acumulado_tiempo_bloquedo=0.0;
   char *shm = NULL, *shm_bandera = NULL;
   long int *shm_cont_prod = NULL,*shm_cont_cons = NULL;
   key_t key,key_semaforo,key_bandera,key_cont_prod,key_cont_cons;
   char *s;
   int semid = 0;

   // Leer datos de la linea de comandos
   s = buffer;
   key = decoder(s,""); // Key del buffer
   key_bandera=decoder(s,"band"); // Key de la bandera
   key_cont_prod=decoder(s,"prod"); // Key del contador de productores
   key_cont_cons=decoder(s,"cons"); // Key del contador de consumidores
   key_semaforo = decoder(s,"sema"); // Key del semaforo
 
   shmid_buf = shmmap(key,&shm,0);
   shmid_bandera = shmmap(key_bandera,&shm_bandera,1);
   shmid_cont_prod = shmmap(key_cont_prod,(char **)&shm_cont_prod,4);
   shmid_cont_cons = shmmap(key_cont_cons,(char **)&shm_cont_cons,4);
   semid = getSemaphore(key_semaforo);

   //Se aumenta el contador de consumidor
   long int id = 0;   
   Wait(semid,3); // Protocolo de entrada
      id = *shm_cont_cons; id++;
      *shm_cont_cons = id;
   Signal(semid,3); // Protocolo de salida
   
   // Se crea un loop hasta que se active la bandera
   char bandera = '0';
   struct timespec antes,despues;
   long int contador_Prod = 0;
   long int contador_Cons = 0;

   initrand();

   int eom = 0;
   while(eom != 1){
      // En el loop se genera el tiempo de espera aleatorio, se espera y
      // se pide el semaforo con wait    Wait(semid);

      printf("\n-----------------------------------------------------\nObtencion de Mensaje\n");

      double espera = dist(*tiempo);      
      clock_gettime ( CLOCK_REALTIME,  &antes ); // Calcular tiempo que esta en el sleep
      sleep((int)espera);
      clock_gettime ( CLOCK_REALTIME,  &despues );
      acumulado_tiempo_esperados += (despues.tv_sec - antes.tv_sec) + (despues.tv_nsec - antes.tv_nsec)*1e-9;


      clock_gettime ( CLOCK_REALTIME,  &antes ); // Calcular tiempo que esta bloqueado por el semaforo
      Wait(semid,0); // Protocolo de entrada
         clock_gettime ( CLOCK_REALTIME,  &despues );
         acumulado_tiempo_bloquedo += (despues.tv_sec - antes.tv_sec) + (despues.tv_nsec - antes.tv_nsec)*1e-9;

         // Se lee la posicion del buffer(indice), leyendo la primera posicion del buffer
         long int indice = (long int)shm[8];
         long int nbuffer = (long int)shm[16];
         s = &shm[SHSIZE]; // A partir del byte SHSIZE esta el array para los mensajes
         s+= MSJSIZE*indice; // Apuntar la posicion para escribir mensaje


         // Se lee el mensaje
         long int *id_prod = (long int*)s;
         time_t *t = (time_t*)&s[8];
         int * aleatorio = (int*)&s[16];

         printf ("Mensaje obtenido exitosamente: \n\t");
         printf("ID: %ld\t", *id_prod);
         printf("TM: %ld\t", *t);
         printf("AL: %d\n", *aleatorio);

         int PID = getpid();
         printf("PID: %d\n", PID);

         if((*id_prod == 0 && *aleatorio == 0 && *t == 0) || (PID%5 == *aleatorio)){
            Signal(semid,0); // Protocolo de salida
            break;
         }

         // Se imprime en consola describiendo la accion realizada, incluyendo el indice y la cantidad de prod/cons
         clock_gettime ( CLOCK_REALTIME,  &antes ); // Calcular tiempo que esta bloqueado por el semaforo
         Wait(semid,2); // Protocolo de entrada
            clock_gettime ( CLOCK_REALTIME,  &despues );
            acumulado_tiempo_bloquedo += (despues.tv_sec - antes.tv_sec) + (despues.tv_nsec - antes.tv_nsec)*1e-9;
            contador_Prod = *shm_cont_prod;
         Signal(semid,2); // Protocolo de salida

         clock_gettime ( CLOCK_REALTIME,  &antes );//Calcular tiempo que esta bloqueado por el semaforo
         Wait(semid,3); // Protocolo de entrada
            clock_gettime ( CLOCK_REALTIME,  &despues );
            acumulado_tiempo_bloquedo += (despues.tv_sec - antes.tv_sec) + (despues.tv_nsec - antes.tv_nsec)*1e-9;
            contador_Cons = *shm_cont_cons;
         Signal(semid,3); // Protocolo de salida

         printf("Indice: %ld\nCantidad de productores: %ld\nCantidad de consumidores: %ld\n-----------------------------------------------------\n", indice, contador_Prod, contador_Cons);

         // Aumentar el indice
         s = &shm[8];      
         indice++;
         if(indice >= nbuffer){
            indice = 0;
         }
         memcpy(s,&indice,sizeof(long int));

      Signal(semid,0); // Protocolo de salida
      numero_mensajes_enviados++;

   }
   
   // Luego de activarse la bandera se debe decrementar el contador de cons
   clock_gettime ( CLOCK_REALTIME,  &antes ); // Calcular tiempo que esta bloqueado por el semaforo      
   Wait(semid,3); // Protocolo de entrada
   clock_gettime ( CLOCK_REALTIME,  &despues );
   acumulado_tiempo_bloquedo += (despues.tv_sec - antes.tv_sec) + (despues.tv_nsec - antes.tv_nsec)*1e-9;

   contador_Cons = *shm_cont_cons; contador_Cons--;
   *shm_cont_cons = contador_Cons;
   Signal(semid,3); // Protocolo de salida

   // Al terminar imprime su informacion

   printf("\n********************************************\nFinal de Ejecucion de Consumidor\n********************************************\n");
   printf("Numero de mensajes recibidos: %ld\nAcumulado de tiempo esperados: %.10lf\nAcumulado de tiempo bloqueado: %.10lf\n********************************************\n", numero_mensajes_enviados, acumulado_tiempo_esperados, acumulado_tiempo_bloquedo);

   // Salir
   return 0;
}
