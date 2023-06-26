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
#define MAX_PROCSS_BKGRND 1000

void limpaTerminalAcsh();

int leLinhaDeComandoAcsh(char* comando);

int contaComandosAcsh(char* linhaDeComando, const char* delimitador);

int trataLinhaDeComandoAcsh(char* linhaDeComando, int qtdMaxArgumentos, pid_t * arraySessoesBG,
    int * sizeArraySessoesBG);

char* separaLinhaEmComandosAcsh(char* linhaDeComando, const char* delimitador);

void executaComandoAcsh(char* comando, char* argumentos[], char * array[], int sizeArray, 
    bool ehComandoUnico, pid_t * arraySessoesBG, int * sizeArraySessoesBG);

int comandoEhForeground(char *argumentos[]);

void executaEmForeground(char ** argumentos);

void executaEmBackground(char ** argumentos, bool ehComandoUnico, pid_t * arraySessoesBG, 
    int * sizeArraySessoesBG, char *array[], int sizeArray);

#endif // ACSH_H