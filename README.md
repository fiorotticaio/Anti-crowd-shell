# Anti-crowd-shell

## Como rodar

```
make
./prog 3
```

## Funcionalidades

O Acsh implementado possui as seguintes funcionalidades:

- [x] Processos em foreground
- [x] Processos em background
- [x] Processos em background são agrupados por novo session id
- [x] Casos não permitidos: (permutações entre: <3  %  cd  exit)
- [x] Limite de comandos (5 comandos)
- [x] CD não cria processo
- [x] Imunidade à SIGUSR1 de processos isolados
- [x] Contágio de SIGUSR1 de processos agrupados
- [x] Sinais default aos processo em background
- [x] exit deve matar todos os processos em background

## Autores

> Caio Fiorotti
> Matheus Schreiber
> Vinicius Amorim