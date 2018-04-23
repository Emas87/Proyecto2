#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>

void initrand() {
	srand((unsigned)time(NULL));
}

double dist(double media){

  double u = rand() / (RAND_MAX + 1.0);

  // Formula = -ln(1-u)/lambda
  // Media = 1/lambda
  // Formula Final = -media * ln(1-u)
  // Donde u es un valor entre 0 y 1 que indica la aleatoridad
  double dist_exp = -media * log(1 - u); 
  
  printf("Tiempo segun Distribucion Exponencial: %f \n", dist_exp);

  return dist_exp;

}

