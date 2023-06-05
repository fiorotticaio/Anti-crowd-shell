# GRUPO:
# Caio Alves Fiorotti
# Matheus Meier Schreiber
# Vinicius Cole de Amorim

QTD_MAX_ARGS = 2

all: clean compile run

compile: acsh
	@ gcc -o prog *.o main.c

acsh:
	@ gcc -c acsh.c

run:
	@ ./prog ${QTD_MAX_ARGS}

val: compile 
	@ valgrind --leak-check=full ./prog ${QTD_MAX_ARGS}

clean:
	@ rm -rf *.o prog