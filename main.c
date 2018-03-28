#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>


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
   key_t key,key_bandera,key_cont_prod,key_cont_cons;
   char *shm = NULL,*shm_cont_prod = NULL,*shm_cont_cons = NULL,*shm_bandera = NULL;
   char *s;
   key = 9876; //key del buffer
   key_bandera=0; //key de la bandera
   key_cont_prod=1; //key del contador de productores
   key_cont_cons=2; //key del contador de consumidores

   shmid_buf = shmmap(key,&shm,SHSIZE);
   shmid_bandera = shmmap(key_bandera,&shm_bandera,SHSIZE);
   shmid_cont_prod = shmmap(key_cont_prod,&shm_cont_prod,SHSIZE);
   shmid_cont_cons = shmmap(key_cont_cons,&shm_cont_cons,SHSIZE);

   memcpy(shm,"hola",4);
   s = shm;
   s =s + 4;
   *s = 0;
   while(*shm != '*'){
      sleep(1);
   }
   
   shmunmap(shmid_buf,shm);
   shmunmap(shmid_bandera,shm_bandera);
   shmunmap(shmid_cont_prod,shm_cont_prod);
   shmunmap(shmid_cont_cons,shm_cont_cons);

   return 0;
}
