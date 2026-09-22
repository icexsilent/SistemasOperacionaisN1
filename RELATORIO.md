# Relatório: Projeto 1

**Grupo:** Grupo 10
**Integrantes (nome e TIA):** Vinicius Rampazi de Oliveira (10426499)
**Turma:**

## 1. Cenário

O simulador representa um serviço de processamento de tarefas. As threads produtoras criam tarefas numeradas de 1 até o total solicitado. As threads consumidoras retiram essas tarefas de uma fila circular compartilhada, limitada por `--capacidade`. Para concluir uma tarefa, cada consumidor precisa manter simultaneamente uma instância do recurso A e uma do recurso B. As quantidades disponíveis são dadas por `--recursos-a` e `--recursos-b`. Os eventos mostram a produção, a disputa pelos recursos e o consumo.

## 2. Seções críticas e mecanismos

- **Fila, índices, quantidade, numeração e fim da produção:** `fila_mutex` protege todas as alterações. `nao_cheia` faz o produtor aguardar enquanto não há espaço; `nao_vazia` faz o consumidor aguardar enquanto não há item. Cada espera usa um laço `while` para reavaliar a condição ao acordar. O mutex também mantém `PRODUZIU` na ordem dos itens e antes do `CONSUMIU` correspondente.
- **Instâncias de A e B:** `recursos_mutex` protege os contadores `disponiveis`. Há uma variável de condição por classe para aguardar liberação. `SOLICITA`, a atualização do contador e `OBTEM` são registrados sob o mesmo mutex, assim como `LIBERA`, para que a ordem do log corresponda ao estado dos recursos.
- **Saída do log:** `log.c` usa seu próprio mutex para impedir que linhas de threads diferentes se misturem. Nenhuma thread segura esse mutex enquanto espera uma condição da fila ou dos recursos.

## 3. Encerramento

O próximo número de item é compartilhado e limitado por `--itens`. Ao produzir o último, o produtor acorda os outros produtores que poderiam aguardar espaço. Cada produtor reduz `produtores_ativos` ao terminar e acorda os consumidores. Um consumidor encerra somente quando a fila está vazia e não há produtores ativos. Assim, itens restantes são drenados e nenhuma thread fica esperando uma produção futura. A thread principal faz `pthread_join` de todas as threads no modo `global` e destrói os mecanismos de sincronização após os joins.

## 4. Deadlock

No modo `ingenua`, dois consumidores podem obter A e depois solicitar B, enquanto outros dois obtêm B e depois solicitam A. O atraso entre as aquisições amplia essa janela. Com dois recursos de cada classe e quatro consumidores, o log termina com `OBTEM` da primeira classe e `SOLICITA` da outra para as threads envolvidas, sem `LIBERA`: há espera circular.

No modo `global`, todos pedem A antes de B. Essa ordem total elimina a condição de **espera circular** de Coffman: não pode haver uma thread segurando B e esperando A. As outras três condições permanecem: cada instância tem posse exclusiva, uma thread pode segurar A enquanto espera B, e um recurso não é retirado à força de sua proprietária.

## 5. Como executar

Em Linux ou ambiente com GCC, POSIX threads e `make`:

```sh
make
./simulador --produtores 3 --consumidores 4 --capacidade 5 --recursos-a 2 --recursos-b 2 --itens 30 --semente 1 --ordem global > simulacao.log
python3 verificar.py simulacao.log --itens 30 --recursos-a 2 --recursos-b 2
timeout 15 ./simulador --produtores 3 --consumidores 4 --capacidade 5 --recursos-a 2 --recursos-b 2 --itens 30 --semente 1 --ordem ingenua > travado.log
python3 verificar.py travado.log --deadlock
```

O `timeout` encerra externamente a demonstração do deadlock; o modo `ingenua` foi feito para travar. Os comandos do modo `global` devem terminar normalmente.

## Evidência de teste

A execução local com MSYS2 UCRT64 aprovou os 12 cenários de `make test`. Em `evidencias/deadlock.log`, `cons-1` e `cons-2` obtiveram A e solicitaram B; `cons-3` e `cons-4` obtiveram B e solicitaram A. Todas as instâncias estavam retidas, sem liberação posterior. O verificador confirmou a espera circular. `evidencias/global.log` contém a execução concluída com os 30 itens. As instruções para repetir os testes no VS Code estão em `TESTES.md`.
