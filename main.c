#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>


#define SHSIZE 100

int main(int argc,char *argv[]){
   int shmid;
   key_t key;
   char *shm;
   char *s;
   struct shmid_ds *shmid_buf;
   key = 9876;
   shmid = shmget(key,SHSIZE,IPC_CREAT | 0666);
   if (shmid < 0){
      perror("shmget");
      exit(1);
   }
   shm = shmat(shmid,NULL,0);
   if (shm == (char *) -1){
      perror("shmmat");
      exit(1);
   }

   memcpy(shm,"hola",4);
   s = shm;
   s =s + 4;
   *s = 0;
   while(*shm != '*'){
      sleep(1);
   }
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

   return 0;
}
