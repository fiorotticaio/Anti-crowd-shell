#include "acsh.h"

void limpaTerminalAcsh() {
  printf("\033[2J\033[H"); // clear
}

int leLinhaDeComandoAcsh(char *comando) {
  printf("acsh> ");
  fgets(comando, TAM_MAX_LINHA_DE_CMD, stdin);
  int qtdComandos = contaComandosAcsh(comando, DELIMITADOR_COMANDO);
  if (qtdComandos > QTD_MAX_COMANDOS+1) {
    fprintf(stderr, "Não é possível executar mais de %d comandos por vez\n", QTD_MAX_COMANDOS);
    return 0;
  }

  /* Tratando quando o usuário usa Ctrl+D que é quando ele envia um EOF para o acsh */
  if (!comando || strlen(comando) == 0) {
    fprintf(stderr, "O comando é uma string NULA, saindo do acsh...\n");
    exit(1);
  }

  comando[strcspn(comando, "\n")] = '\0'; // Remover o caractere de nova linha

  return 1;
}

int contaComandosAcsh(char *linhaDeComando, const char *delimitador) {
  int count = 0;
  int seqLen = strlen(delimitador);
  int cmdLen = strlen(linhaDeComando);

  if (seqLen > cmdLen) return 0;

  /* Percorre a linha de comando contando quando delimitadores tem */
  for (int i = 0; i <= cmdLen - seqLen; i++) {
    if (strncmp(linhaDeComando + i, delimitador, seqLen) == 0)
      count++;
  }

  return count + 2;
}

/* Verifica o usuário quer rodar varios programas em foreground */
static int ehLinhaDeComandoValida(char *linhaDeComando) {
  if (strstr(linhaDeComando, DELIMITADOR_COMANDO) != NULL &&
      strstr(linhaDeComando, "%") != NULL
  ) { // Tem <3 e %
    return 0;
  } else if (strstr(linhaDeComando, DELIMITADOR_COMANDO) == NULL) { // Não tem <3
    return -1; // Para mostrar que é um comando único
  }
  else return 1;
}

int trataLinhaDeComandoAcsh(char *linhaDeComando, int qtdMaxArgumentos, pid_t * arraySessoesBG, int * sizeArraySessoesBG) {

  pid_t acshSid = getsid(getpid()); //TODO: tirar isso (nao ta sendo usado, acredito que tenha sido para teste)
  fprintf(stderr, "ACSH SID: %d\n", acshSid);
  
  bool ehComandoUnico = false; // Para tratar o sinal SIGUSR1
  bool precisaDeNovaSID = true; // Para mudar o session id do comandos agrupados

  int rtn = ehLinhaDeComandoValida(linhaDeComando);
  
  // Linha de comando com <3 e %
  if (rtn == 0) { 
    printf("Nao eh possivel executar varios comandos em foreground!\n");
    return rtn;
  } else if (rtn == -1) ehComandoUnico = true;

  char *token, *token2, *argumentos[qtdMaxArgumentos];
  token = separaLinhaEmComandosAcsh(linhaDeComando, DELIMITADOR_COMANDO); // Separa a linha de comando com o delimitador
  
  
  // loop para passar por todos os comandos de uma mesma linha (possível uso de <3)
  while(token!=NULL){
    char cmd[TAM_MAX_CMD];
    int i = 0, j = 0;
    char *array[100];

    /* Transformar a linha em palavras */
    token2 = strtok(token, DELIMITADOR_ARG);
    while (token2 != NULL) {
      array[i] = strdup(token2);
      token2 = strtok(NULL, DELIMITADOR_ARG);
      i++;
    }
    strcpy(cmd, array[0]); // A primeira palavra é o comando em si

    /* As outras palavras são os argumentos */
    for (j = 0; j < i; j++)
      argumentos[j] = array[j];
    argumentos[j] = NULL; // O último argumento é NULL
    

    // // verificando se é necessário criar uma nova sessão para o grupo de programas
    // if(strcmp(argumentos[j-1], "%" != 0) && precisaDeNovaSID) { // Se forem vários comandos em background, cria um mesmo session id para todos
    //   pid_t nova_secao = setsid();
    //   if(nova_secao < 0) {
    //     fprintf(stderr, "Erro na criação da sessão em background.\n");
    //     exit(1);
    //   }
    //   precisaDeNovaSID=false;
    // }
    

    /* Executando o comando */
    executaComandoAcsh(cmd, argumentos, array, i, ehComandoUnico, arraySessoesBG, sizeArraySessoesBG);
    token = separaLinhaEmComandosAcsh(NULL, DELIMITADOR_COMANDO); 

    /* Liberando a memória alocada para cada palavra do comando */
    for (j = 0; j < i; j++) free(array[j]);
  }
  
  return rtn;
}

char *separaLinhaEmComandosAcsh(char *linhaDeComando, const char *delimitador) {
  static char *token = NULL;
  if (linhaDeComando != NULL && strcmp(linhaDeComando, "\n") != 0)
    token = linhaDeComando;
  if (token == NULL)
    return NULL;

  char *posDelimitador = strstr(token, delimitador);
  if (posDelimitador == NULL) {
    char *tokenAtual = token;
    token = NULL;
    return tokenAtual;
  }

  *posDelimitador = '\0';
  char *tokenAtual = token;
  token = posDelimitador + strlen(delimitador);
  return tokenAtual;
}

static void sigusr1Handler(int sinal) {
  pid_t sid = getsid(getpid());
  kill(-sid, SIGTERM); // Envia o sinal SIGTERM para todos os processos na sessão
}

void executaComandoAcsh(char *comando, char *argumentos[], char *array[], int sizeArray,
    bool ehComandoUnico, pid_t * arraySessoesBG, int * sizeArraySessoesBG) {

  /* Caso seja exit, saia do programa */
  if (strcmp(comando, "exit") == 0) {

    // Matando processos em background
    for (int j = 0; j < *sizeArraySessoesBG; j++) killpg(arraySessoesBG[j], SIGKILL);
    
    // Liberando memória
    for (int j = 0; j < sizeArray; j++) free(array[j]);

    // Saindo
    exit(0);

  } else if (strcmp(comando, "cd") == 0) {
    /*
      Caso o comando seja cd, nenhum processo filho é criado,
      apenas alteramos o diretório de trabalho do processo pai
    */
    int i = 0;
    while (argumentos[i] != NULL) i++;

    const char *homeDir = getenv("HOME"); // Vai pra pasta home caso não tenha argumentos
    if (!argumentos[1]) chdir(homeDir);

    chdir(argumentos[i - 1]);
    return;
  }

  if (comandoEhForeground(argumentos)) 
    executaEmForeground(argumentos);
  else 
    executaEmBackground(argumentos, ehComandoUnico, arraySessoesBG, sizeArraySessoesBG, array, sizeArray);
}

int comandoEhForeground(char *argumentos[]) {
  int i = 0;
  while (argumentos[i] != NULL) i++;

  /* Se o último argumento for "%", executar em foreground */
  if (i > 0 && strcmp(argumentos[i - 1], "%") == 0) {
    argumentos[i - 1] = NULL;
    return 1;
  }

  return 0;
}

void executaEmBackground(char ** argumentos, bool ehComandoUnico, pid_t * arraySessoesBG, 
    int * sizeArraySessoesBG, char *array[], int sizeArray) {

  // Criando um novo processo que sera usado para agrupar
  // os processos em background em uma mesma sessão (session leader)
  // já que os seus filhos vão herdar sua sessão
  pid_t pid_nova_secao = fork(); 

  if (pid_nova_secao < 0) {
    fprintf(stderr, "Erro ao criar processo principal de background\n");
    exit(1);

  // Processo filho (session leader)
  } else if (pid_nova_secao == 0) { 
    pid_t nova_secao = setsid();
    
    pid_t acshSid = getsid(getpid()); //TODO: tirar isso (nao ta sendo usado, acredito que tenha sido para teste)
    fprintf(stderr, "CHILD START SID: %d\n", acshSid);
        
    /* Implementa o tratador do SIGUSR1 caso não seja um comando único */
    if (!ehComandoUnico) {
      signal(SIGUSR1, sigusr1Handler); // Tratador personalizado
    } else {
      signal(SIGUSR1, SIG_IGN); // Ignora sinal
    }

    // Processos netos (session members)
    int pid = fork();
    
    if (pid<0) {
      fprintf(stderr, "Erro ao criar processo em background\n");

    } else if (pid == 0) {
      // Redirecionar entrada, saída e saída de erro para /dev/null,
      // para executar em background

      pid_t childSID = getsid(getpid()); //TODO: tirar isso (nao ta sendo usado, acredito que tenha sido para teste)
      fprintf(stderr, "CHILD SID: %d\n", childSID);
      
      int devnull = open("/dev/null", O_RDWR);
      dup2(devnull, STDIN_FILENO);
      dup2(devnull, STDOUT_FILENO);
      // dup2(devnull, STDERR_FILENO);

      close(devnull);
      
      /* Executar o comando no processo filho */
      execvp(argumentos[0], argumentos);

      /* Se execvp retornar, houve um erro */
      fprintf(stderr, "Erro ao executar o comando: %s\n", argumentos[0]);
      exit(1); 

    } else {
      int status;
      waitpid(pid, &status, 0); 
      if(WIFEXITED(status)) {
        if(WEXITSTATUS(status) == 1) fprintf(stderr, "Programa encerrado com EXIT 1.\n");
        else if(WEXITSTATUS(status) == 2) fprintf(stderr, "Programa encerrado com EXIT 2.\n");
      }
    }

    for (int j = 0; j < sizeArray; j++) free(array[j]);
    exit(0);
  } else {
    arraySessoesBG[(*sizeArraySessoesBG)++] = getsid(getpid());
  }
}

void executaEmForeground(char ** argumentos) {
  pid_t pid = fork(); // Cria um processo separado para executar o comando

  if (pid < 0) {
    printf("Erro ao criar processo\n");
    exit(1);

  } else if (pid == 0) { // Processo filho
    signal(SIGINT, SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
    signal(SIGTSTP, SIG_DFL);

    //FIXME: consertar esse sinal handler
    // ta bugado <-
    
    /* Executar o comando no processo filho */
    execvp(argumentos[0], argumentos);

    /* Se execvp retornar, houve um erro */
    fprintf(stderr, "Erro ao executar o comando: %s\n", argumentos[0]);
    exit(1); 

  } else if (pid > 0) {
    int status;
    
    waitpid(pid, &status, 0); // Esperar o processo filho terminar
    
    //TODO: adicionar tratamento para SIGUSR1

    if(WIFEXITED(status)) {
      if(WEXITSTATUS(status) == 1) printf("Programa encerrado com EXIT 1.\n");
      else if(WEXITSTATUS(status) == 2) printf("Programa encerrado com EXIT 2.\n");
    }
  }
}