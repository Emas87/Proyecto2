# Proyecto2

ocupamos: 
funcion para parsear argumento de entradas robusta, que verifique que buffer no sea mayor a 9, ni tenga ningun caracter que no pertenezca a el alfabeta en minuscula, y asegurar segundo arguemnto sea de tipo int(Oscar)
funcion string_to_key(Oscar)(check)

Creador:
shmget de buffer(check)
Semaforo (check)
shmget de bandera de terminacion(check)
shmget de contador productor(check)
shmget de contador de consumir(check)

Product/Consumidor:
Funcion que calcule tiempos aleatorios para la espera, distribucion exponencial
funcion mandar mensajes
funcion leer mensajes

all:
transformar el buffer para administracion circular? se ocupa un puntero, podemos utilizar el mimso buffer para el puntero, le asiganamos la primer posicion al puntero, antes de leer y escribir se revisa el puntero
getSemaphore(check)
