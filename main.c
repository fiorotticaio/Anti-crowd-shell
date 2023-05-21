#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define TAM_MAX_LINHA_DE_CMD 2000
#define DELIMITADOR_COMANDO " <3 "

static void limpaTerminal() {
  printf("\033[2J\033[H"); // clear
}

void leComando(char* comando) {
  printf("acsh> ");
  fgets(comando, TAM_MAX_LINHA_DE_CMD, stdin);
  comando[strcspn(comando, "\n")] = '\0';  // Remover o caractere de nova linha
}

void executaComando(char* comando) {
  if (strcmp(comando, "exit") == 0) {
    exit(0); // Finaliza o programa
  }

  // Implemente aqui a lógica para executar o comando
  // Você pode usar funções como execvp() ou system()
  // Exemplo básico:
  printf("%s\n", comando);
  system(comando);
}

int main(int argc, char **argv) {
  char linhaDeComando[TAM_MAX_LINHA_DE_CMD];
  char* token;

  limpaTerminal();
  while (1) {
    leComando(linhaDeComando);

    token = strtok(linhaDeComando, DELIMITADOR_COMANDO); // Separa a linha de comando com o delimitador
    while (token != NULL) {
      executaComando(token);
      token = strtok(NULL, DELIMITADOR_COMANDO);
    }
  }

  return 0;
}