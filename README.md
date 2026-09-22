# Projeto 1: Simulador multithread

Sistemas Operacionais · 2026.2

Este repositório é a especificação completa e o ponto de partida.

Versão da especificação: **v1**. Correções e esclarecimentos são publicados
aqui e registrados no `CHANGELOG.md`; acompanhe com `git pull`.

## Do que se trata

Um sistema operacional coordena tarefas que rodam ao mesmo tempo e disputam
recursos finitos: memória, descritores de arquivo, acesso a disco, tempo de
CPU. As decisões de projeto envolvidas nessa coordenação aparecem em qualquer
software que use concorrência, de um servidor web a um pipeline de dados.

Aqui vocês constroem uma versão reduzida desse problema. Threads produtoras
geram itens, threads consumidoras os processam, uma estrutura compartilhada de
capacidade limitada fica entre elas, e cada item consumido exige duas classes
de recurso escasso ao mesmo tempo. O simulador registra tudo o que acontece
num log de eventos, e é sobre esse log que a corretude do programa é
avaliada.

Uma parte central do projeto é o modo `ingenua`, que trava de propósito.
Vocês vão reproduzir o deadlock, apontar a evidência dele no log e explicar
qual das condições necessárias a correção de vocês elimina.

## Objetivos de aprendizagem

Ao final do projeto vocês devem ser capazes de:

1. Identificar seções críticas num programa concorrente e escolher o mecanismo
   de sincronização adequado a cada uma, justificando a escolha.
2. Implementar produtor e consumidor sobre uma estrutura de capacidade
   limitada, com bloqueio correto nas condições de cheia e vazia.
3. Reproduzir um deadlock de forma controlada, reconhecer as quatro condições
   de Coffman no próprio código e eliminar uma delas.
4. Projetar um encerramento ordenado, em que nenhuma thread fica bloqueada
   esperando um evento que não vai ocorrer.
5. Sustentar afirmações sobre corretude com evidência de execução.
6. Trabalhar em equipe sobre um repositório Git compartilhado, com histórico
   de commits que registra a contribuição de cada integrante.

## Compilar e rodar

Para Windows e VS Code, consulte [TESTES.md](TESTES.md). A rotina `make test` compila e valida os dois modos automaticamente.

```
make
./simulador --produtores 3 --consumidores 4 --capacidade 5 \
            --recursos-a 2 --recursos-b 2 --itens 30 \
            --semente 1 --ordem global > simulacao.log
```

O log de eventos vai para `stdout`. Diagnóstico e erro vão para `stderr`, e é
por isso que o redirecionamento acima produz um arquivo limpo.

## Verificar antes de entregar

O `verificar.py` é o mesmo script usado na correção, e afere os 5,0 pontos de
funcionamento da rubrica. Rode-o antes de entregar: o que ele acusar aqui vai
ser acusado na correção.

```
python3 verificar.py simulacao.log --itens 30 --recursos-a 2 --recursos-b 2
```

Um detalhe que costuma reprovar código correto: a ordem das linhas no log
também é um invariante. Se você decrementa o contador de recursos dentro da
região protegida mas emite o `OBTEM` depois de soltar o mutex, duas threads
podem logar fora de ordem e o log vai acusar mais instâncias em uso do que o
limite, ainda que o programa nunca tenha ultrapassado o limite de fato. Logue
o evento enquanto ainda segura o mutex que protege o contador.

Ele confere: formato e vocabulário do log, toda thread com `CRIADA` e
`ENCERRADA`, itens produzidos e consumidos batendo sem duplicatas, nenhuma
classe de recurso em uso acima do limite, `CONSUMIU` sempre com as duas classes
em mãos, e nenhum recurso retido ao fim.

Para o modo travado:

```
timeout 15 ./simulador ... --ordem ingenua > travado.log
python3 verificar.py travado.log --deadlock
```

Aqui o script procura espera circular: pelo menos duas threads segurando uma
classe e aguardando outra, em ciclo. Um programa que simplesmente não termina
não passa neste teste.

## O que já está pronto

| Arquivo | Situação |
|---|---|
| `Makefile` | pronto. Compila tudo que estiver em `src/` |
| `src/args.c`, `src/args.h` | pronto. Não altere os nomes dos parâmetros |
| `src/log.c`, `src/log.h` | pronto. Não altere o formato da linha |
| `src/main.c` | esqueleto com os `TODO` |
| `verificar.py` | pronto. É o script da correção |

Organize o código em quantos arquivos `.c` fizerem sentido dentro de `src/`.

## Avaliação (10 pontos)

| Critério | Pontos |
|---|---|
| Funcionamento e invariantes do log | 5,0 |
| Modo deadlock: reprodução, correção e evidência no log | 1,0 |
| Relatório | 1,0 |
| Apresentação e respostas individuais | 3,0 |

Os 3,0 de apresentação são individuais: cada integrante responde sobre o
projeto como um todo, não apenas sobre a parte que escreveu.

## Trabalho em equipe

Cada integrante deve contribuir com os seus próprios commits, feitos da sua
própria conta. O histórico é parte da avaliação e é conferido com:

```
git shortlog -sn --all
```

Um integrante sem commits de autoria própria recebe zero na parcela de
funcionamento, ainda que o grupo entregue o projeto completo. Combinem a
divisão no início e façam commits pequenos ao longo do desenvolvimento: todo
o trabalho reunido num commit único na véspera reduz a nota do grupo,
independentemente do resultado.

## Entrega

Além do código: `RELATORIO.md` e `USO_DE_IA.md`, ambos preenchidos.
