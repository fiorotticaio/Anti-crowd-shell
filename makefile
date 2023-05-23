# GRUPO:
# Caio Alves Fiorotti
# Matheus Meier Schreiber
# Vinicius Cole de Amorim

QTD_MAX_ARGS = 5

all: clean compile run

compile:
	@ gcc -o prog main.c

run:
	@ ./prog ${QTD_MAX_ARGS}

val: compile 
	@ valgrind --leak-check=full ./prog ${QTD_MAX_ARGS}

clean:
	@ rm -rf *.o prog