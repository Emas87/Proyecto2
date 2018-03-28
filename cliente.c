#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define handle_error(msg) \
   do { perror(msg); exit(EXIT_FAILURE); } while (0)


int main(int argc,char *argv[]){
   void *addr;
   //unsigned int *dir;
   //int dir;
   char dir[16];
   size_t length = 10;
   char writes[length];
   FILE *fd;
   fd = fopen("dir.txt","r");
   
   fscanf(fd,"%s",dir);
   long int num = (long int)strtol(dir, NULL, 16);
   addr = num;
   printf("map id: %p\n",addr);

   mmap(addr, length, PROT_READ | PROT_WRITE , MAP_SHARED | MAP_ANONYMOUS ,-1, 0);
   if (addr == MAP_FAILED){
      handle_error("mmap");
   }

   memcpy(writes,addr,length);
   printf("esto dice writes: %s\n",writes);
   munmap(addr, length);
   fclose(fd);
   return 0;
}
