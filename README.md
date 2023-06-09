# Anti-crowd-shell

## IMPLEMENTAÇÕES PENDENTES

Nesas seção marque as funcionalidades já implementadas e adicione as que ainda faltam ser implementadas

- [ ] não está sendo tratado o sinal caso um filho tenha algum erro.
    Por exemplo, se o filho faz um exit(1), o pai não está captando com WEXITSTATUS
    para saber o que houve...

## BUGS

Nessa seção marque os problemas que foram resolvidos e adicione os problemas ainda a resolver

- [x] Faltando 1 free no valgrind
- [ ] Processos filho devem ser todos de uma mesma seção, diferente do pai 
- [x] o sinal Ctrl+D envia o caracter EOF para o acsh, e ai entra em loop infinito
- [ ] usar o comando `ls acsh.c acsh.h acsh.o main.c %` faz com que o acsh saia 
    sem nenhuma mensagem de log nem nada aparentemente esse comportamento acontece
    quando você passa mais de 3 argumentos para o ls
- [ ] sinais enviados ao acsh quando há processo em foreground devem passar pelo
    proceso antes, mas estão sendo enviados direto ao acsh
- [ ] o comando sleep não está sendo executado em background

## EXTRA

Adicionar mais comentários no código