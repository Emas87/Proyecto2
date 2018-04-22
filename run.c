#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <regex.h>
#include <math.h>
#include "dist.h"

void funcion(int modo,char *buffer_name,long int* size,double*average_time,int argc,char *argv[]){

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

   int i,j = 0; // iteradores

   // No se ingresan argumentos
   if(argc == 1){
      printf("No hay ningun argumento\n");
      exit(-1);
   }

   // Revision de argumentos
   for(i = 0;i < argc - 1; i++){

      // En caso de que algun argumento sea solo un guion "-"
      if(strcmp(argv[i+1],"-") == 0){
         printf("Opcion - invalida\n");
         exit(-1);
      }

      // Verificar que el argumento empieza con "-"
      status = regexec(&re,argv[i+1],0,NULL,0);

      // Las opciones se ingresan primero que los parametros
      if(!status){
         if(i%2!=0){
            printf("ERROR: Formato incorrecto en argumento %s\n", argv[i+1] );
            printf("El formato adecuado es: Programa -opcion parametro\n");
            exit(-1);
         }
         cant_opc += 1;
         opciones[i] = argv[i+1]; // Si es una opcion, o sea de la forma "-" y lo que sea, se almacena como una opcion
         parametros[i] = "";

      } else { // Los parametros se ingresan seguido de las opciones 
         if(i%2==0){
            printf("ERROR: Formato incorrecto en argumento %s\n", argv[i+1] );
            printf("El formato adecuado es: Programa -opcion parametro -opcion parametro\n");
            exit(-1);
         }
         cant_param += 1;
         parametros[i] = argv[i+1]; // Se almacenan los parametros
         opciones[i] = "";

      }

   }

   // Revision de que cada opcion tenga un parametro asociado y viceversa
   if(cant_opc != cant_param){
      printf("Cantidad de datos no concuerda\n");
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
                  printf("Se repite opcion %s\n", opcion_actual[i]);
                  exit(-1);
               }
            }
         }
      }
   }


   // Revisar que tipo de opciones se ingresaron 
   for(i = 0;i < argc - 1 ; i++){
      // Buffer
      if(strcmp(opciones[i],"-buffer") == 0){
         buffer_name = parametros[i+1];
         for(j = 0; j < strlen(buffer_name); j++){
            if(!(buffer_name[j]>='a' && buffer_name[j]<='z')){
              printf("ERROR: La opcion %s solo acepta letras minusculas sin numeros\n", opciones[i]);
              exit(-1);
            } 
         }
         printf("Buffer: %s\n", buffer_name);
      }

      // Size 
      else if(strcmp(opciones[i],"-size") == 0){
         char *size_s = parametros[i+1];
         if(strlen(size_s)>8){
           printf("Tamano no puede ser mayor a 8 digitos\n");
           exit(-1);
         }
         for(j = 0; j < strlen(size_s); j++){
            if(!(size_s[j]>='0' && size_s[j]<='9')){
              printf("ERROR: La opcion %s solo acepta numeros enteros\n", opciones[i]);
              exit(-1);
            } 
         }
         size = atoi(size_s);
         printf("Size: %d\n", size);
      } 

      // Time
      else if(strcmp(opciones[i],"-time") == 0){
         char *average_time_s = parametros[i+1];
         if(strlen(average_time_s)>10){
           printf("Tiempo no puede ser mayor a 10 digitos\n");
           printf("El punto decimal se cuenta como un digito\n");
           exit(-1);
         }
         for(j = 0; j < strlen(average_time_s); j++){
            if(!(average_time_s[j]>='0' && average_time_s[j]<='9')){
              if(!(average_time_s[j]==46)){ // Es un punto
                printf("ERROR: La opcion %s solo acepta numeros de punto flotante utilizando punto como separador\n", opciones[i]);
                exit(-1);
              }
            } 
         }
         average_time = atof(average_time_s);
         printf("Time: %.10lf\n", average_time);
      } 

      // Opcion default
      else if(strcmp(opciones[i],"") == 0){
         // Do nothing
      } 

      // Opcion invalida, se intenta usar una opcion invalida -d, -x, etc.
      else { 
         printf("Opcion %s invalida\n",opciones[i]);         exit(-1);
      }
   }

}

int main(int argc,char *argv[]){
   int modo= 0;
   char *buffer;
   long int* tamano;
   double* tiempo;
   funcion( modo,buffer, tamano,tiempo, argc,argv);

   //testeo de variables de salida

   //


   //dist(average_time);
   

   // Creador -> Nombre del Buffer, Tamano en entradas para msjs

   // Productor -> Nombre del Buffer, Parametro media tiempo aleatorio

   // Consumidor -> Nombre del Buffer, Parametro media tiempo aleatorio 

   // Finalizador -> No args

  
   return 0;
}
