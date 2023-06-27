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
#define DELIMITADOR_COMANDO "<3"
#define DELIMITADOR_ARG " "
#define QTD_MAX_COMANDOS 5
#define MAX_PROCSS_BKGRND 1000

void limpaTerminalAcsh();
int leLinhaDeComandoAcsh(char* comando);
int ehLinhaDeComandoValida(char *linhaDeComando);
int contaComandosAcsh(char* linhaDeComando, const char* delimitador);
int trataLinhaDeComandoAcsh(char* linhaDeComando, int qtdMaxArgumentos, pid_t * arraySessoesBG, int * sizeArraySessoesBG);

void executaCD(char * argumentos[]);
void executaEmForeground(char * argumentos[], int qtdMaxArgumentos);
void executaExit(pid_t * arraySessoesBG, int * sizeArraySessoesBG);
void executaEmBackground(char * argumentos[], int qtdMaxArgumentos, bool ehComandoUnico, pid_t * arraySessoesBG, int * sizeArraySessoesBG);

#endif // ACSH_H