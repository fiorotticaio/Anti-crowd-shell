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