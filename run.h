#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <regex.h>

void parser(int modo, char *buffer_name[], long int* size, double*average_time, int argc, char *argv[]){

   int status;
   regex_t re;
   status = regcomp(&re,"^-[[:alnum:]]",0);
   if(status){
      printf("No se compilo la expresion regular ^-[[:alnum:]]+");  
      exit(1);
   }

   char * opciones[argc -1]; // Puntero con las opciones
   char * parametros[argc -1]; // Puntero con los parametros
   int cant_opc = 0; // Cantidad de opciones especificadas
   int cant_param = 0; // Cantidad de parametros especificados
   int i,j = 0; // Iteradores

   // No se ingresan argumentos
   if(argc == 1){
      printf("ERROR: No se ingreso ningun argumento\n");
      exit(-1);
   }

   // Revision de argumentos
   for(i = 0;i < argc - 1; i++){

      // En caso de que algun argumento sea solo un guion "-"
      if(strcmp(argv[i+1],"-") == 0){
         printf("ERROR: Opcion \"-\" invalida\n");
         exit(-1);
      }

      // Verificar que el argumento empieza con "-"
      status = regexec(&re,argv[i+1],0,NULL,0);

      // Las opciones se ingresan primero que los parametros
      if(!status){
         if(i%2!=0){
            printf("ERROR: Formato incorrecto en argumento %s\n", argv[i+1] );
            printf("El formato adecuado es: \nPrograma -opcion parametro -opcion parametro\n");
            exit(-1);
         }
         cant_opc += 1;
         opciones[i] = argv[i+1]; // Si es una opcion, o sea de la forma "-" y lo que sea, se almacena como una opcion
         parametros[i] = "";

      } else { // Los parametros se ingresan seguido de las opciones 
         if(i%2==0){
            printf("ERROR: Formato incorrecto en argumento %s\n", argv[i+1] );
            printf("El formato adecuado es: \nPrograma -opcion parametro -opcion parametro\n");
            exit(-1);
         }
         cant_param += 1;
         parametros[i] = argv[i+1]; // Se almacenan los parametros
         opciones[i] = "";

      }

   }

   // Revision de que cada opcion tenga un parametro asociado y viceversa
   if(cant_opc != cant_param){
      printf("ERROR: Se ingreso una cantidad diferente de opciones y parametros \nLa cantidad de datos no concuerda:\n");
      printf("Opciones: %d\n",cant_opc);
      printf("Parametros: %d\n",cant_param);
      exit(-1);
   }

   // Revision de repeticion de opciones
   char * opcion_actual[argc -1]; 
   for(i = 0;i < argc - 1; i++){
      opcion_actual[i] = opciones[i];
      if(strcmp(opcion_actual[i],"") != 0) {
         for(j = 0;j < argc - 1; j++){
            if (i != j){
               if(strcmp(opcion_actual[i],opciones[j]) == 0){
                  printf("ERROR: Se repite la opcion %s\n", opcion_actual[i]);
                  exit(-1);
               }
            }
         }
      }
   }

   // Revisar que tipo de opciones se ingresaron 
   int buffer_on = 0;
   int size_on = 0;
   int time_on = 0;

   for(i = 0;i < argc - 1 ; i++){
      // Buffer
      if(strcmp(opciones[i],"-b") == 0){
         buffer_on = 1;
         *buffer_name = parametros[i+1];
         if(strlen(*buffer_name)>100){
           printf("ERROR: Buffer no puede ser mayor a 100 digitos\n");
           exit(-1);
         }
         for(j = 0; j < (int)strlen(*buffer_name); j++){
            if(!(((*buffer_name)[j]>='a' && (*buffer_name)[j]<='z') || ((*buffer_name)[j]>='A' && (*buffer_name)[j]<='Z') || ((*buffer_name)[j]>='0' && (*buffer_name)[j]<='9') || ((*buffer_name)[j]==95))){
              printf("ERROR: Nombre de Buffer Incorrecto\nUsar Mayusculas, Minusculas, Numeros y Guion Bajo\n");
              exit(-1);
            }
         }
         printf("Buffer: %s\n", *buffer_name);
      }

      // Size 
      else if(strcmp(opciones[i],"-s") == 0){
         size_on = 1;
         char *size_s = parametros[i+1];
         if(strlen(size_s)>8){
           printf("ERROR: Tamano no puede ser mayor a 8 digitos\n");
           exit(-1);
         }
         for(j = 0; j < (int)strlen(size_s); j++){
            if(!(size_s[j]>='0' && size_s[j]<='9')){
              printf("ERROR: La opcion %s solo acepta numeros enteros\n", opciones[i]);
              exit(-1);
            } 
         }
         *size = atoi(size_s);
         printf("Tamano Buffer: %ld\n", *size);
      } 

      // Time
      else if(strcmp(opciones[i],"-t") == 0){
         time_on = 1;
         char *average_time_s = parametros[i+1];
         if(strlen(average_time_s)>10){
           printf("ERROR: Tiempo no puede ser mayor a 10 digitos\n");
           printf("NOTA: El punto decimal se cuenta como un digito\n");
           exit(-1);
         }
         int point_detected = 0;
         for(j = 0; j < (int)strlen(average_time_s); j++){
            if(average_time_s[j]==46){
               point_detected += 1 ;
	       if(point_detected>1){
                  printf("ERROR: Se ingreso dos veces el punto \".\" en la opcion %s\n", opciones[i]);
                  exit(-1);
               }
            }
            if(!((average_time_s[j]>='0' && average_time_s[j]<='9') || (average_time_s[j]==46))){
                printf("ERROR: La opcion %s solo acepta numeros reales utilizando punto como separador\n", opciones[i]);
                exit(-1);
            } 
         }

         double av_time = atof(average_time_s);
         *average_time = av_time;
         printf("Tiempo Promedio: %.10lf\n", *average_time);
      } 

      // Opcion default
      else if(strcmp(opciones[i],"") == 0){
         // Esta opcion es configurada por el algoritmo, no hacer nada.
      } 

      // Opcion invalida, se intenta usar una opcion invalida -d, -x, etc.
      else { 
         printf("ERROR: La opcion %s es invalida\n",opciones[i]);
         exit(-1);
      }
   }

   if(modo == 0){ // Creador
      if(buffer_on == 0 || size_on == 0){
	 printf("ERROR: Faltan argumentos\n");
	 printf("Creador debe tener nombre (-b) y tamano (-s) del buffer como argumentos de entrada\n");
         exit(-1);
      } else if (time_on == 1){
         printf("ERROR: Sobran argumentos\n");
         printf("Creador no necesita argumentos de tiempo promedio (-t)\n");
         exit(-1);
      }
   }
   else if(modo == 1){ // Consumidor-Productor
      if(buffer_on == 0 || time_on == 0){
	 printf("ERROR: Faltan argumentos\n");
	 printf("Consumidor o Productor deben tener nombre (-b) del buffer y tiempo promedio (-t) como argumentos de entrada\n");
         exit(-1);
      } else if (size_on == 1){
         printf("ERROR: Sobran argumentos\n");
         printf("Consumidor o Productor no necesitan argumentos de tamano (-s)\n");
         exit(-1);
      }
   }
   else if(modo == 2){ // Finalizador
      if(buffer_on == 0){
	 printf("ERROR: Faltan argumentos\n");
	 printf("Finalizador debe tener nombre (-b) del buffer como argumento de entrada\n");
         exit(-1);
      } else if (time_on == 1 || size_on == 1){
         printf("ERROR: Sobran argumentos\n");
         printf("Finalizador no necesita argumentos de tiempo promedio (-t) ni de tamano (-s)\n");
         exit(-1);
      }
   }
}
