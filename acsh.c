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

  // Tratando quando o usuário usa Ctrl+D que é quando ele envia um EOF para o acsh
  if (!comando || strlen(comando) == 0) {
    fprintf(stderr, "O comando é uma string NULA, saindo do acsh...\n");
    exit(1);
  }

  // Remover o caractere de nova linhas
  comando[strcspn(comando, "\n")] = '\0'; 

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

  return count + 3;
}

/* Verifica qual caso de uso está sendo feito com a linha de comando atual */
int ehLinhaDeComandoValida(char *linhaDeComando) {
  bool changedirectory = strstr(linhaDeComando, "cd") != NULL;      // Tem cd (change directory)
  bool exit = strstr(linhaDeComando, "exit") != NULL;               // Tem exit
  bool unico = strstr(linhaDeComando, DELIMITADOR_COMANDO) == NULL; // Comando único
  bool background = strstr(linhaDeComando, "%") == NULL;            // Comando background (não tem %)
  bool foreground = strstr(linhaDeComando, "%") != NULL;            // Comando foreground (tem %)  
  
  bool invalido1 = strstr(linhaDeComando, DELIMITADOR_COMANDO) != NULL && foreground;       // Tem <3 e %
  bool invalido2 = changedirectory && strstr(linhaDeComando, DELIMITADOR_COMANDO) != NULL;  // Tem cd e <3
  bool invalido3 = exit && strstr(linhaDeComando, DELIMITADOR_COMANDO) != NULL;             // Tem exit e <3  
  bool invalido4 = exit && changedirectory;                                                 // Tem cd e exit

  if (invalido1 || invalido2 || invalido3 || invalido4) return 0;
  else if (changedirectory) return 1;
  else if (exit) return 2;
  else if (background && unico) return 3;
  else if (background && !unico) return 4;
  else if (foreground) return 5;
  else return -1;
}

int trataLinhaDeComandoAcsh(char *linhaDeComando, int qtdMaxArgumentos, pid_t * arraySessoesBG, int * sizeArraySessoesBG) {
  int i=0;
  bool ehComandoUnico = false; 
  char * argumentos[qtdMaxArgumentos];

  int rtn = ehLinhaDeComandoValida(linhaDeComando);

  // Saida inesperada (comando inválido não esperado)
  if (rtn == -1){
    fprintf(stderr, "Erro inesperado!\n");
    exit(1);
  }

  // Linha de comando com (<3 e %) ou com (<3 e cd) ou com (<3 e exit) ou com (cd e exit)
  if (rtn == 0) { 
    fprintf(stderr, "Nao eh possivel executar varios comandos em foreground!\n");
    return 1;  
  
  // Varios comandos em background (situação que usa o DELIMITADOR_COMANDO)
  } else if (rtn == 4) {
    char * token = strtok(linhaDeComando, DELIMITADOR_COMANDO);
    while (token!=NULL) {
      argumentos[i++] = token;
      token = strtok(NULL, DELIMITADOR_COMANDO);
    }
    executaEmBackground(argumentos, i, ehComandoUnico, arraySessoesBG, sizeArraySessoesBG);
  
  
  // Todas as outras situações usam o delimitador DELIMITADOR_ARG
  } else {

    char * token = strtok(linhaDeComando, DELIMITADOR_ARG);
    while (token!=NULL) {
      argumentos[i++] = token;
      token = strtok(NULL, DELIMITADOR_ARG);
    }

    argumentos[i] = NULL;
    
    // Linha de comando com cd
    if (rtn == 1) { 
      executaCD(argumentos);   

    // Linha de comando com exit
    } else if (rtn == 2) {
      executaExit(arraySessoesBG, sizeArraySessoesBG);

    // Comando em background mas é único
    } else if (rtn == 3) {
      ehComandoUnico = true;
      executaEmBackground(argumentos, qtdMaxArgumentos, ehComandoUnico, arraySessoesBG, sizeArraySessoesBG);

    // Comando em foreground
    } else if (rtn == 5) {
      executaEmForeground(argumentos, qtdMaxArgumentos);
    }
  }

  return rtn; // se rtn == 0 -> acsh exit
}


static void sigusr1Handler(int sinal) {
  pid_t sid = getsid(getpid());
  kill(-sid, SIGTERM); // Envia o sinal SIGTERM para todos os processos na sessão
}

void executaExit(pid_t * arraySessoesBG, int * sizeArraySessoesBG){
  // Matando processos em background
  for (int j = 0; j < *sizeArraySessoesBG; j++) killpg(arraySessoesBG[j], SIGKILL);
  
  // Saindo
  exit(0);
}

void executaCD(char * argumentos[]){
  // Caso o comando seja cd, nenhum processo filho é criado,
  // apenas alteramos o diretório de trabalho do processo pai
  int i = 0;
  while (argumentos[i] != NULL) i++;

  // Vai pra pasta home caso não tenha argumentos
  const char *homeDir = getenv("HOME"); 
  if (!argumentos[1]) chdir(homeDir);

  chdir(argumentos[i - 1]);
  return;
}

void executaEmBackground(char * argumentos[], int qtdMaxArgumentos, bool ehComandoUnico, pid_t * arraySessoesBG, int * sizeArraySessoesBG) {

  // Criando um novo processo que sera usado para agrupar
  // os processos em background em uma mesma sessão (session leader)
  // já que os seus filhos vão herdar sua sessão
  pid_t sidNovaSecao = fork();

  if (sidNovaSecao < 0) {
    fprintf(stderr, "Erro ao criar processo principal de background\n");
    exit(1);


  // Processo filho (session leader)
  } else if (sidNovaSecao == 0) { 
    pid_t novaSecao = setsid();
    if (novaSecao < 0) {
      fprintf(stderr, "Erro ao criar nova seção\n");
      exit(1);
    }

    // Se não for comando único (comandos agrupados)
    if (!ehComandoUnico) {
      signal(SIGUSR1, sigusr1Handler); // Tratador personalizado

      // Passando por um loop de comandos (comandos agrupados)
      int i=0;
      for(i=0;i<qtdMaxArgumentos;i++){
        int j=0;
        if (argumentos[i]==NULL) break;
        
        // Destrinchando cada comando em um vetor (variável array) com argumentos
        char * token = strtok(argumentos[i], DELIMITADOR_ARG);
        char * array[TAM_MAX_LINHA_DE_CMD];
        while (token != NULL) {
          array[j++] = token;
          token = strtok(NULL, DELIMITADOR_ARG);
        }
        array[j] = NULL;

        // Iniciando um fork para executar o código
        int pid = fork();
      
        if (pid<0) {
          fprintf(stderr, "Erro ao criar processo em background\n");

        // Processo neto (que vai executar o código)
        } else if (pid == 0) {
          // Redirecionar entrada e saída para /dev/null,
          // para executar em background
          int devnull = open("/dev/null", O_RDWR);
          dup2(devnull, STDIN_FILENO);
          dup2(devnull, STDOUT_FILENO);
          // dup2(devnull, STDERR_FILENO);
          close(devnull);
          
          // Executar o comando no processo filho
          execvp(array[0], array);

          // Se execvp retornar, houve um erro
          fprintf(stderr, "Erro ao executar o comando: %s\n", array[0]);
          exit(1); 
        } 
      }
        
    // Se for comando único
    } else {
      signal(SIGUSR1, SIG_IGN); // Ignora sinal

      int devnull = open("/dev/null", O_RDWR);
      dup2(devnull, STDIN_FILENO);
      dup2(devnull, STDOUT_FILENO);
      // dup2(devnull, STDERR_FILENO);

      close(devnull);

      // Executar o comando no processo filho
      execvp(argumentos[0], argumentos);

      // Se execvp retornar, houve um erro
      fprintf(stderr, "Erro ao executar o comando: %s\n", argumentos[0]);
      exit(1); 
    }

    // Terminando o session leader após tudo acabar (esperando os outros terminarem)
    int status;
    pid_t pid = waitpid(-1, &status, 0);
    
    // Quando um processo sai naturalmente 
    if (WEXITSTATUS(status) == 1) fprintf(stderr, "Programa encerrado com EXIT 1.\n");
    
    // Quando um processo recebe um sinal e é terminado, então todos os outros processos da sessão também são terminados
    if (WIFSIGNALED(status)) killpg(sidNovaSecao, SIGKILL);

    exit(0);
    
  } else {
    arraySessoesBG[(*sizeArraySessoesBG)++] = sidNovaSecao;
  }
}

void executaEmForeground(char * argumentos[], int qtdMaxArgumentos) {
  
  // Retirando o % do array de argumentos
  for (int i=0; i<qtdMaxArgumentos; i++) {
    if (strcmp(argumentos[i], "%")==0) {
      argumentos[i] = NULL;
      break;
    }
  }

  // Cria um processo separado para executar o comando
  pid_t pid = fork(); 

  if (pid < 0) {
    printf("Erro ao criar processo\n");
    exit(1);

  // Processo filho
  } else if (pid == 0) { 
    signal(SIGINT, SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
    signal(SIGTSTP, SIG_DFL);

    // Executar o comando no processo filho
    execvp(argumentos[0], argumentos);

    // Se execvp retornar, houve um erro
    fprintf(stderr, "Erro ao executar o comando: %s\n", argumentos[0]);
    exit(1); 

  } else if (pid > 0) {
    int status;
    
    waitpid(pid, &status, 0); // Esperar o processo filho terminar

    if(WIFEXITED(status)) {
      if(WEXITSTATUS(status) == 1) printf("Programa encerrado com EXIT 1.\n");
      else if(WEXITSTATUS(status) == 2) printf("Programa encerrado com EXIT 2.\n");
    }
  }
}