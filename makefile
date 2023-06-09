# GRUPO:
# Caio Alves Fiorotti
# Matheus Meier Schreiber
# Vinicius Cole de Amorim

QTD_MAX_ARGS = 2

all: clean compile

compile: acsh
	@ gcc -o prog *.o 

acsh:
	@ gcc -c *.c

run:
	@ ./prog ${QTD_MAX_ARGS}

val: clean compile 
	@ valgrind --leak-check=full ./prog ${QTD_MAX_ARGS}

clean:
	@ rm -rf *.o prog


# gcc -c -o bin/executa_prompt.o src/executa_prompt.c
# gcc -c -o bin/main.o src/main.c
# gcc -c -o bin/processa_prompt.o src/processa_prompt.c
# gcc -c -o bin/recebe_prompt.o src/recebe_prompt.c
# gcc -o trab1 bin/executa_prompt.o bin/main.o bin/processa_prompt.o bin/recebe_prompt.o