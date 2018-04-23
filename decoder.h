#include <ctype.h>
#define BNAME 100

long int decoder(char *name_b,const char* add){
   char name[BNAME+10];
   memset(name,0,BNAME+10);

   strcpy(name, add);
   strcat(name, name_b);


   int i;
   int value = 0;
   int len = strlen(name);
   char str[BNAME];
   char conversion[BNAME];
   strcpy(conversion, "");
   for(i=0;i<len;i++){
      value = toupper(name[i])-48;
      sprintf(str, "%d", value);
      strcat(conversion,str);
   }

   long int result = strtol(conversion, NULL, 10);
   //printf("Buffer KEY: 0x%08x\n", (unsigned int)result);

   return result;

}
