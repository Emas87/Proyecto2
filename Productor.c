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


#define SHSIZE 24

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
   int MSJSIZE = sizeof(long int) + sizeof(time_t) + sizeof(int);//se define asi para que pueda ser portable
   long int numero_mensajes_enviados=0;
   double acumulado_tiempo_esperados=0,acumulado_tiempo_bloquedo=0;
   char *shm = NULL, *shm_bandera = NULL;
   long int *shm_cont_prod = NULL,*shm_cont_cons = NULL;
   key_t key,key_semaforo,key_bandera,key_cont_prod,key_cont_cons;
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

   shmid_buf = shmmap(key,&shm,SHSIZE+(MSJSIZE)*5);
   shmid_bandera = shmmap(key_bandera,&shm_bandera,1);
   shmid_cont_prod = shmmap(key_cont_prod,(char **)&shm_cont_prod,4);
   shmid_cont_cons = shmmap(key_cont_cons,(char **)&shm_cont_cons,4);
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
   time_t antes,despues;
   long int contador_Prod = 0;
   long int contador_Cons = 0;

	srand((unsigned)time(NULL));
   while(bandera != '1'){
      // En el loop se genera el tiempo de espera aleatorio, se espera y
      // se pide el semaforo con wait    Wait(semid);
      time ( &antes );//Calcular tiempo que esta en el sleep
      sleep(1);
      time ( &despues );
      acumulado_tiempo_esperados+=despues-antes;


      time ( &antes );//Calcular tiempo que esta bloqueado por el semaforo
      Wait(semid,0); //protocolo de entrada
         time ( &despues );
         acumulado_tiempo_bloquedo+=despues-antes;

         // se lee la posicion del buffer(indice), leyendo la primera posicion del buffer
         long int indice = (long int)shm[0];
         long int nbuffer = (long int)shm[16];
         s = &shm[16];
         memcpy(s,&nbuffer,sizeof(long int));

         s = &shm[SHSIZE]; // a partir del byte SHSIZE esta el array para los mensajes
         s+= MSJSIZE*indice;//apuntar la posicion para escribir mensaje

         // escribe el mensaje(id del prod,fecha y hora,llave aleatoria entre 0 y 4)
         //long int,time_t,int
         memcpy(s,&id,sizeof(long int));
         s+= sizeof(long int);
         time_t rawtime;
         time ( &rawtime );
         memcpy(s,&rawtime,sizeof(time_t));
         s+= sizeof(time_t);
         int aleatorio = rand()%5;
         memcpy(s,&aleatorio,sizeof(int));

         // se imprime en consola describiendo la accion realizada,incluyendo el indice y la cantidad de prod/consum
         time ( &antes );//Calcular tiempo que esta bloqueado por el semaforo
         Wait(semid,2); //protocolo de entrada
            time ( &despues );
            acumulado_tiempo_bloquedo+=despues-antes;

            contador_Prod = *shm_cont_prod;
         Signal(semid,2); //protocolo de salida

         time ( &antes );//Calcular tiempo que esta bloqueado por el semaforo
         Wait(semid,3); //protocolo de entrada
            time ( &despues );
            acumulado_tiempo_bloquedo+=despues-antes;

            contador_Cons = *shm_cont_cons;
         Signal(semid,3); //protocolo de salida

         printf("Se escribio mensaje:\nIndice: %ld\nCantidad de productores: %ld\nCantidad de consumidores: %ld\n",indice,contador_Prod,contador_Cons);
      
         //Aumentar el indice
         indice++;
         if(indice >= nbuffer){
            indice = 0;
         }
         memcpy(shm,&indice,sizeof(long int));

      Signal(semid,0); //protocolo de salida
      numero_mensajes_enviados++;

      //Verificar bandera
      time ( &antes );//Calcular tiempo que esta bloqueado por el semaforo      
      Wait(semid,1); //protocolo de entrada
         time ( &despues );
         acumulado_tiempo_bloquedo+=despues-antes;
         bandera = *shm_bandera;
      Signal(semid,1); //protocolo de salida
   }
   
   // Luego de activarse la bandera se debe decrementar el contador de prod
   time ( &antes );//Calcular tiempo que esta bloqueado por el semaforo      
   Wait(semid,2); //protocolo de entrada
      time ( &despues );
      acumulado_tiempo_bloquedo+=despues-antes;
      contador_Prod = *shm_cont_prod; contador_Prod--;
      *shm_cont_prod = contador_Prod;
   Signal(semid,2); //protocolo de salida

   //Al terminar imprime su informacion
   printf("Termino productor\nId: %ld\nNumero de mensajes enviados: %ld\nAcumulado de tiempo esperados: %lf\nAcumulado de tiempo de espera:%lf\n",id,numero_mensajes_enviados,acumulado_tiempo_esperados,acumulado_tiempo_bloquedo);

   // Salir
   return 0;
}
