#ifndef ACSH_H
#define ACSH_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define TAM_MAX_LINHA_DE_CMD 2000
#define TAM_MAX_CMD 1000
#define DELIMITADOR_COMANDO " <3 "
#define DELIMITADOR_ARG " "
#define QTD_MAX_COMANDOS 5

void limpaTerminalAcsh();

int leLinhaDeComandoAcsh(char* comando);

int contaComandosAcsh(char* linhaDeComando, const char* delimitador);

int trataLinhaDeComandoAcsh(char* linhaDeComando, int qtdMaxArgumentos);

char* separaLinhaEmComandosAcsh(char* linhaDeComando, const char* delimitador);

void executaComandoAcsh(char* comando, char* argumentos[], char * array[], int sizeArray, bool ehComandoUnico);

int executaEmForegroundAcsh(char* comando, char* argumentos[]);


#endif // ACSH_H