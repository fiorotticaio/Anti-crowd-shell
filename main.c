#include "acsh.h"

void signalHandler(int sinal) {
  if (sinal == SIGINT) 
    printf("\nNão adianta me enviar o sinal por Ctrl-C. Estou vacinado!!\n");
  else if (sinal == SIGQUIT)
    printf("\nNão adianta me enviar o sinal por Ctrl-\\. Estou vacinado!!\n");
  else if (sinal == SIGTSTP) 
    printf("\nNão adianta me enviar o sinal por Ctrl-Z. Estou vacinado!!\n");
  else {
    printf("\nNão deveria ter recebido esse sinal: %d\n", sinal);
  }

  if (getsid(getpid()) == getpid()) kill(getpid(), SIGINT);
}

int main(int argc, char **argv) {
  /* Adicionando o tratador de sinais */
  struct sigaction sa;
  sa.sa_handler = signalHandler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGQUIT, &sa, NULL);
  sigaction(SIGTSTP, &sa, NULL);
  
  /* Declaração de variáveis importantes */
  int qtdMaxArgumentos = argv[1] != NULL ? atoi(argv[1]) : 3; 
  char linhaDeComando[TAM_MAX_LINHA_DE_CMD];
  
  // limpaTerminalAcsh(); // Limpa o terminal antes de começar

  int rtn = 0;

  while (1) {
    leLinhaDeComandoAcsh(linhaDeComando);

    int qtdComandos = contaComandosAcsh(linhaDeComando, DELIMITADOR_COMANDO);
    if (qtdComandos > QTD_MAX_COMANDOS) {
      printf("Não é possível executar mais de %d comandos por vez\n", QTD_MAX_COMANDOS);
      continue;
    }

    rtn = trataLinhaDeComandoAcsh(linhaDeComando, qtdMaxArgumentos);

    if (rtn == 0) // acsh> exit
      break;
  }
  return 0;
}