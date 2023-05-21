# GRUPO:
# Caio Alves Fiorotti
# Matheus Meier Schreiber
# Vinicius Cole de Amorim

all: clean compile run

compile:
	@ gcc -o prog main.c

run:
	@ ./prog 

clean:
	@ rm -rf *.o prog