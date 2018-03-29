#include <ctype.h>
int decoder(char *name){

   int i;
   int value = 0;
   int len = strlen(name);
   char str[1024];
   char conversion[1024];
   strcpy(conversion, "");
   for(i=0;i<len;i++){
      value = toupper(name[i])-48;
      sprintf(str, "%d", value);
      strcat(conversion,str);
   }

   long int result = strtol(conversion, NULL, 10);
   printf("Buffer KEY: %ld\n", result);

   return value;

}
