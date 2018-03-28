#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define handle_error(msg) \
   do { perror(msg); exit(EXIT_FAILURE); } while (0)

int main(int argc,char *argv[]){
   void *addr;
   size_t length = 10;
   char writes[length];
   FILE *fd;

   addr =  mmap(NULL, length, PROT_READ | PROT_WRITE , MAP_SHARED | MAP_ANONYMOUS ,-1, 0);
   if (addr == MAP_FAILED){
      handle_error("mmap");
   }
   mlock(addr,length);
   fd = fopen("dir.txt","w+");
   fprintf(fd,"%p",addr);
   fflush(fd);
   printf("map id: %p\n",addr);
   char out = 'i';
   memcpy(addr,"hola",length);
   while(out != 'o'){
      printf("digite algo\n");
      scanf("%c",&out);
   }
   munlock(addr,length);
   fclose(fd);
   munmap(addr, length);
   return 0;
}
