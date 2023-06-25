# Anti-crowd-shell

## IMPLEMENTAÇÕES PENDENTES

Nessas seção marque as funcionalidades já implementadas e adicione as que ainda faltam ser implementadas

- [x] Outra particularidade do acsh é que o sinal SIGUSR1 é um sinal muito perigoso e contagioso! Se um dos processos de background morre devido ao SIGUSR1, os demais processos “irmãos” que se encontram na mesma sessão (ou seja, que foram criados na mesma linha de comando) devem morrer de forma coletiva, devido ao mesmo sinal
- [x] se um processo de background for criado isoladamente, como neste exemplo: acsh > comando1 ... ele não poderá morrer devido ao sinal SIGUSR1… nunca!
- [X] acsh> comando1 % Neste exemplo acima, o acsh deverá criar um processo em foreground e pertencendo à mesma sessão do acsh . Neste caso, o prompt só será novamente exibido ao final da execução do processo de foreground criado
- [x] se tiver <3 não pode ter %
- [?] se houver um processo de foreground rodando, então este deve receber qualquer sinal gerado via “Ctrl-...” e executar o tratamento default.
- [ ] Quanto aos processos de background, estes não receberão nenhum sinal gerado via “Ctrl-...”, como é esperado. Mas, obviamente, se alguém enviar um sinal para um dos processos de background via chamada “kill”, esse sinal deverá ser entregue normalmente ao processo, o qual deverá executar o tratamento default (lembrando daquela pequena exceção do SIGUSR1 ... no caso de um processo em background que tenha sido criado isoladamente, ele se encontra protegido em sua respectiva sessão, então ele não morre devido ao SIGUSR1 … )


## BUGS

Nessa seção marque os problemas que foram resolvidos e adicione os problemas ainda a resolver

- [ ] não está sendo tratado o sinal caso um filho tenha algum erro.
      Por exemplo, se o filho faz um exit(1), o pai não está captando com WEXITSTATUS
      para saber o que houve...
- [x] Faltando 1 free no valgrind
- [x] Processos filho devem ser todos de uma mesma seção, diferente do pai
- [x] o sinal Ctrl+D envia o caracter EOF para o acsh, e ai entra em loop infinito
- [ ] usar o comando `ls acsh.c acsh.h acsh.o main.c %` faz com que o acsh saia
      sem nenhuma mensagem de log nem nada aparentemente esse comportamento acontece
      quando você passa mais de 3 argumentos para o ls
- [X] sinais enviados ao acsh quando há processo em foreground devem passar pelo
      processo antes, mas estão sendo enviados direto ao acsh
- [X] o comando sleep não está sendo executado em background
- [ ] o sinal Ctrl+D não é tratado caso seja utilizado juntamento com outros textos,
      e ele faz com que infinitos processos sejam criados

## EXTRA

Adicionar mais comentários no código
