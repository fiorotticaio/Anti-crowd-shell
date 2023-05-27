#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define TAM_MAX_LINHA_DE_CMD 2000
#define TAM_MAX_CMD 1000
#define DELIMITADOR_COMANDO " <3 "
#define DELIMITADOR_ARG " "

// TODO: josex falou que tem que modularizar
/*========== Cabeçalho das funções ==========*/
static void limpaTerminal();
void leLinhaDeComando(char* comando);
void sinalCtrlC(int sinal);
char* separaLinhaEmComandos(char* linhaDeComando, const char* delimitador);
void executaComando(char* comando, char* argumentos[], char * array[], int sizeArray);
int executaEmForeground(char* comando, char* argumentos[]);

/*========== Main ==========*/
int main(int argc, char **argv) {
  int qtdMaxArgumentos = argv[1] != NULL ? atoi(argv[1]) : 3; 
  char linhaDeComando[TAM_MAX_LINHA_DE_CMD], *token, *token2, *argumentos[qtdMaxArgumentos];
  limpaTerminal();

  pid_t pid = getpid();
  pid_t spid = getsid(pid);
  // printf("PID do acsh: %d\n", pid);
  // printf("PID da sessão do acsh: %d\n", spid);
  
  // chamando o 'listener' de sinais'
  signal(SIGINT, sinalCtrlC);

  while (1) {
    leLinhaDeComando(linhaDeComando);

    // FIXME: se setar outra sessão, a detecção de sinais fica em segundo plano
    //        e então ele nao mostra a mensagem na tela
    setsid(); // Cria uma nova sessão para o processo filho

    token = separaLinhaEmComandos(linhaDeComando, DELIMITADOR_COMANDO); // Separa a linha de comando com o delimitador
    while (token != NULL) {
      char cmd[TAM_MAX_CMD];
      int i = 0, j = 0;
      char * array[100];
      // /* Transformar a linha em palavras */
      token2 = strtok(token, DELIMITADOR_ARG);
      while (token2 != NULL) {
        array[i] = strdup(token2);
        token2 = strtok(NULL, DELIMITADOR_ARG);
        i++;
      }
      strcpy(cmd, array[0]); // A primeira palavra é o comando em si
      /* As outras palavras são os argumentos */
      for (j = 0; j < i; j++) argumentos[j] = array[j];
      argumentos[j] = NULL; // O último argumento é NULL
      executaComando(cmd, argumentos, array, i);
      token = separaLinhaEmComandos(NULL, DELIMITADOR_COMANDO); // Próximo comando
      /* Liberar a memória alocada para cada palavra do comando */
      for (j = 0; j < i; j++) free(array[j]); 
    }
  }

  return 0;
}


/*========== Implementação das funções ==========*/
static void limpaTerminal() {
  printf("\033[2J\033[H"); // clear
}

void leLinhaDeComando(char* comando) {
  printf("acsh> ");

  fgets(comando, TAM_MAX_LINHA_DE_CMD, stdin);
  comando[strcspn(comando, "\n")] = '\0';  // Remover o caractere de nova linha
}

void sinalCtrlC(int sinal){
  fprintf(stderr, "\nNão adianta me enviar o sinal por Ctrl-... Estou vacinado!!\nacsh> ");
  if (getsid(getpid()) == getpid()) kill(getpid(), SIGINT);
}

// TODO: Entender essa função obscura do nosso amigo Chat 
char* separaLinhaEmComandos(char* linhaDeComando, const char* delimitador) {
  static char* token = NULL;
  if (linhaDeComando != NULL && strcmp(linhaDeComando, "\n")!=0) token = linhaDeComando;
  if (token == NULL) return NULL;

  char* posDelimitador = strstr(token, delimitador);
  if (posDelimitador == NULL) {
    char* tokenAtual = token;
    token = NULL;
    return tokenAtual;
  }

  *posDelimitador = '\0';
  char* tokenAtual = token;
  token = posDelimitador + strlen(delimitador);
  return tokenAtual;
}

void executaComando(char* comando, char* argumentos[], char * array[], int sizeArray) {
  if (strcmp(comando, "exit") == 0) {
    /*
      Caso seja exit, saia do programa
    */
    for (int j = 0; j < sizeArray; j++) free(array[j]); 
    exit(0); 

  } else if (strcmp(comando, "cd") == 0) {
  /* 
    Caso o comando seja cd, nenhum processo filho é criado,
    apenas alteramos o diretório de trabalho do processo pai
  */
    int i = 0;
    while (argumentos[i] != NULL) i++;
    
    const char* homeDir = getenv("HOME"); // Vai pra pasta home caso não tenha argumentos
    if (!argumentos[1]) chdir(homeDir);

    chdir(argumentos[i-1]);
    return;
  } 

  /* Cria um processo separado para executar o comando */
  pid_t pid = fork();
  if (pid < 0) {
    printf("Erro ao criar processo\n");
    exit(1);

  } else if (pid == 0) { // Processo filho
    pid_t spid = getsid(getpid());
    // printf("PID do processo filho: %d\n", getpid());
    // printf("PID da sessão do processo filho: %d\n", spid);

    if (!executaEmForeground(comando, argumentos)) {
      /* 
        Redirecionar entrada, saída e saída de erro para /dev/null, 
        para executar em background 
      */
      int devnull = open("/dev/null", O_RDWR);
      dup2(devnull, STDIN_FILENO);
      dup2(devnull, STDOUT_FILENO);
      dup2(devnull, STDERR_FILENO);
      close(devnull);
    }

    /* Executar o comando no processo filho */
    execvp(comando, argumentos);

    /* Se execlp retornar, houve um erro */
    fprintf(stderr, "Erro ao executar o comando: %s\n", comando);
    exit(1);

  } else { // Processo pai
    int status;
    waitpid(pid, &status, 0); // Esperar o processo filho terminar
  }
}

int executaEmForeground(char* comando, char* argumentos[]) {
  int i = 0;
  while (argumentos[i] != NULL) i++;

  /* Comandos internos devem ser executados em foreground sempre */
  if (strcmp(comando, "exit") == 0 || strcmp(comando, "cd") == 0) return 1;

  /* Se o último argumento for "%", executar em foreground */
  if (i > 0 && strcmp(argumentos[i-1], "%") == 0) {
    argumentos[i-1] = NULL;
    return 1;
  }

  return 0;
}
