all: Productor Consumidor Creador Finalizador

Productor : Productor.c
	gcc Productor.c -o Productor -g -W -lm

Consumidor : Consumidor.c
	gcc Consumidor.c -o Consumidor -g -W -lm

Creador : Creador.c
	gcc Creador.c -o Creador -g -W

Finalizador : Finalizador.c
	gcc Finalizador.c -o Finalizador -g -W

clean:
	rm Productor Consumidor Creador Finalizador
