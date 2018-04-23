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


#define SHSIZE 24 // 8 bytes para index de prod,8 bytes para index de consumidores, 8 bytes para tamano del buffer y 8 bytes para llevar el ultimo ID de consumidores

int shmmap(key_t key,char **shm,int shsize){
   int shmid = shmget(key,shsize,0666);
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
   int modo= 1;
   char buff[100];
   char *buffer; buffer = buff;
   long int tam;
   long int* tamano;tamano = &tam;
   double tiem;
   double* tiempo;tiempo = &tiem;

   printf("********************************************\nEjecucion de Productor\n********************************************\n");

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

   // Se obtiene el id del productor leyendo la shm de contador de productor
   // Se usa el semaforo antes de leerlo, se asigna el id, se escribe y se hace Signal(semid);
   long int id = 0;
   Wait(semid,2); // Protocolo de entrada
      id = *shm_cont_prod; id++;
      *shm_cont_prod = id;
   Signal(semid,2); // Protocolo de salida
   
   
   // Se crea un loop hasta que se active la bandera
   char bandera = '0';
   struct timespec antes,despues;
   long int contador_Prod = 0;
   long int contador_Cons = 0;

   initrand();

   while(bandera != '1'){
      // En el loop se genera el tiempo de espera aleatorio, se espera 
      // se pide el semaforo con wait    Wait(semid);

      printf("\n-----------------------------------------------------\nGeneracion de Mensaje\n");

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
         long int indice = (long int)shm[0];
         long int nbuffer = (long int)shm[16];
         s = &shm[16]; // Se va escribir el tamano, por alguna razon si no se reeescribe el finalizador no logra ver esto
         memcpy(s,&nbuffer,sizeof(long int));

         s = &shm[SHSIZE]; // A partir del byte SHSIZE esta el array para los mensajes
         s+= MSJSIZE*indice; // Apuntar la posicion para escribir mensaje

         // Verificar bandera para no sobreescribir ningun mensaje que haya escrito el Finalizador
         clock_gettime ( CLOCK_REALTIME,  &antes ); // Calcular tiempo que esta bloqueado por el semaforo      
         Wait(semid,1); // Protocolo de entrada
            clock_gettime ( CLOCK_REALTIME,  &despues );
            acumulado_tiempo_bloquedo += (despues.tv_sec - antes.tv_sec) + (despues.tv_nsec - antes.tv_nsec)*1e-9;
            bandera = *shm_bandera;
         Signal(semid,1); // Protocolo de salida
         if(bandera == '1'){
            Signal(semid,0); // Protocolo de salida
            break;
         }
         // Escribe el mensaje(id del prod, fecha y hora, llave aleatoria entre 0 y 4)
         memcpy(s,&id,sizeof(long int));
         s+= sizeof(long int);
         time_t rawtime;
         time ( &rawtime );
         memcpy(s,&rawtime,sizeof(time_t));
         s+= sizeof(time_t);
         int aleatorio = rand()%5;
         memcpy(s,&aleatorio,sizeof(int));

         printf ("Mensaje generado exitosamente\n");

         // Se imprime en consola describiendo la accion realizada,incluyendo el indice y la cantidad de prod/consum
         clock_gettime ( CLOCK_REALTIME,  &antes ); // Calcular tiempo que esta bloqueado por el semaforo
         Wait(semid,2); // Protocolo de entrada
            clock_gettime ( CLOCK_REALTIME,  &despues );
            acumulado_tiempo_bloquedo += (despues.tv_sec - antes.tv_sec) + (despues.tv_nsec - antes.tv_nsec)*1e-9;

            contador_Prod = *shm_cont_prod;
         Signal(semid,2); // Protocolo de salida

         clock_gettime ( CLOCK_REALTIME,  &antes ); // Calcular tiempo que esta bloqueado por el semaforo
         Wait(semid,3); // Protocolo de entrada
            clock_gettime ( CLOCK_REALTIME,  &despues );
            acumulado_tiempo_bloquedo += (despues.tv_sec - antes.tv_sec) + (despues.tv_nsec - antes.tv_nsec)*1e-9;

            contador_Cons = *shm_cont_cons;
         Signal(semid,3); // Protocolo de salida

         printf("Indice: %ld\nCantidad de productores: %ld\nCantidad de consumidores: %ld\n-----------------------------------------------------\n", indice, contador_Prod, contador_Cons);
      
         // Aumentar el indice
         indice++;
         if(indice >= nbuffer){
            indice = 0;
         }
         memcpy(shm,&indice,sizeof(long int));

      Signal(semid,0); // Protocolo de salida
      numero_mensajes_enviados++;

      // Verificar bandera
      clock_gettime ( CLOCK_REALTIME,  &antes ); // Calcular tiempo que esta bloqueado por el semaforo      
      Wait(semid,1); // Protocolo de entrada
         clock_gettime ( CLOCK_REALTIME,  &despues );
         acumulado_tiempo_bloquedo += (despues.tv_sec - antes.tv_sec) + (despues.tv_nsec - antes.tv_nsec)*1e-9;
         bandera = *shm_bandera;
      Signal(semid,1); // Protocolo de salida
   }
   
   // Luego de activarse la bandera se debe decrementar el contador de prod
   clock_gettime ( CLOCK_REALTIME,  &antes ); // Calcular tiempo que esta bloqueado por el semaforo      
   Wait(semid,2); // Protocolo de entrada
      clock_gettime ( CLOCK_REALTIME,  &despues );
      acumulado_tiempo_bloquedo += (despues.tv_sec - antes.tv_sec) + (despues.tv_nsec - antes.tv_nsec)*1e-9;
      contador_Prod = *shm_cont_prod; contador_Prod--;
      *shm_cont_prod = contador_Prod;
   Signal(semid,2); // Protocolo de salida

   // Al terminar imprime su informacion
   printf("\n********************************************\nFinal de Ejecucion de Productor\n********************************************\n");
   printf("Id: %ld\nNumero de mensajes enviados: %ld\nAcumulado de tiempo esperados: %.10lf\nAcumulado de tiempo bloqueado: %.10lf\n********************************************\n", id, numero_mensajes_enviados, acumulado_tiempo_esperados, acumulado_tiempo_bloquedo);

   // Salir
   return 0;
}
