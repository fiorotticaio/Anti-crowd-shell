#include "acsh.h"

void limpaTerminalAcsh() {
  printf("\033[2J\033[H"); // clear
}

int leLinhaDeComandoAcsh(char* comando) {
  printf("acsh> ");

  fgets(comando, TAM_MAX_LINHA_DE_CMD, stdin);
  
  int qtdComandos = contaComandosAcsh(comando, DELIMITADOR_COMANDO);
  if (qtdComandos > QTD_MAX_COMANDOS) {
    fprintf(stderr, "Não é possível executar mais de %d comandos por vez\n", QTD_MAX_COMANDOS);
    return 0;
  }

  // Tratando quando o usuário usa Ctrl+D que é quando ele envia um EOF para o acsh
  if (!comando || strlen(comando) == 0) {
    fprintf(stderr, "O comando é uma string NULA, saindo do acsh...\n");
    exit(1);
  }

  comando[strcspn(comando, "\n")] = '\0';  // Remover o caractere de nova linha

  return 1;
}

int contaComandosAcsh(char* linhaDeComando, const char* delimitador) {
  int count = 0;
  int seqLen = strlen(delimitador);
  int cmdLen = strlen(linhaDeComando);

  if (seqLen > cmdLen)
    return 0;

  /* Percorre a linha de comando contando quando delimitadores tem */
  for (int i = 0; i <= cmdLen - seqLen; i++) {
    if (strncmp(linhaDeComando + i, delimitador, seqLen) == 0)
      count++;
  }

  return count+2;
}

int trataLinhaDeComandoAcsh(char* linhaDeComando, int qtdMaxArgumentos) {
  
  pid_t pid = fork(); // Cria um processo separado para executar o(s) comando(s)

  if (pid < 0) {
    printf("Erro ao criar processo\n");
    exit(1);

  } else if (pid == 0) { // Processo filho

    pid_t novoSIdFilho = setsid(); // Criar uma nova sessão para o processo filho

    char *token, *token2, *argumentos[qtdMaxArgumentos];
    token = separaLinhaEmComandosAcsh(linhaDeComando, DELIMITADOR_COMANDO); // Separa a linha de comando com o delimitador

    while (token != NULL) {
      char cmd[TAM_MAX_CMD];
      int i = 0, j = 0;
      char * array[100];

      /* Transformar a linha em palavras */
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

      executaComandoAcsh(cmd, argumentos, array, i);
      token = separaLinhaEmComandosAcsh(NULL, DELIMITADOR_COMANDO); // Próximo comando

      /* Liberar a memória alocada para cada palavra do comando */
      for (j = 0; j < i; j++) free(array[j]); 

      
    }

  } else { // Processo pai
    int status;
    waitpid(pid, &status, 0); // Esperar o processo filho terminar
    return status;
  }
}

char* separaLinhaEmComandosAcsh(char* linhaDeComando, const char* delimitador) {
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

void executaComandoAcsh(char* comando, char* argumentos[], char * array[], int sizeArray) {

  if (strcmp(comando, "exit") == 0) {
    /* Caso seja exit, saia do programa */
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

  pid_t pid = fork(); // Cria um processo separado para executar o comando

  if (pid < 0) {
    printf("Erro ao criar processo\n");
    exit(1);

  } else if (pid == 0) { // Processo filho

    if (!executaEmForegroundAcsh(comando, argumentos)) {
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
    exit(1); //TODO: esse sinal não está sendo tratado pelo processo PAI

  } else if (pid > 0) {
    int status;
    waitpid(pid, &status, 0); // Esperar o processo filho terminar
  }
}

int executaEmForegroundAcsh(char* comando, char* argumentos[]) {
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