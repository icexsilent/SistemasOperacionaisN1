# Como compilar e testar

## Pelo VS Code no Windows

O ambiente desta máquina está em `E:\Tools\msys64`, com GCC, POSIX threads, GNU make e Python.

1. Abra esta pasta no VS Code.
2. Pressione **Ctrl+Shift+B** para compilar.
3. Use **Terminal > Executar Tarefa > Testar simulador** para compilar e rodar todos os cenários.

Para abrir um terminal com as ferramentas disponíveis, crie um novo terminal **MSYS2 UCRT64**. Nesse terminal:

```sh
make
make test
```

Pelo PowerShell, a mesma rotina pode ser executada com:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\msys2.ps1 -Acao testar
```

As ações disponíveis no script são `compilar`, `testar`, `demonstrar` e `limpar`. A ação `demonstrar` gera `simulacao.log` e o verifica.

## Preparar outra máquina Windows

Instale o [MSYS2](https://www.msys2.org/docs/installer/) e abra o terminal **MSYS2 UCRT64**. Atualize os pacotes e instale as dependências:

```sh
pacman -Syu
# Se o terminal fechar durante a atualização, abra-o novamente.
pacman -Syu --needed mingw-w64-ucrt-x86_64-gcc make python
```

O script PowerShell procura o MSYS2 em `E:\Tools\msys64`, `C:\msys64` e `E:\msys64`. Para outro local, defina `MSYS2_ROOT` ou passe `-Msys2Root`. O perfil de terminal em `.vscode/settings.json` usa o caminho desta máquina; ajuste-o se instalar em outro local.

## Linux

Com GCC, make e Python 3 instalados:

```sh
make test
```

## O que é verificado

A rotina `testar.py` executa o programa compilado e chama o `verificar.py` fornecido pela disciplina:

- Oito cenários em modo `global`: configuração mínima, exemplo do enunciado, mais produtores que itens, mais consumidores que itens, fila ampla, escassez de A, escassez de B e concorrência com fila de capacidade 1.
- Quatro execuções de deadlock: duas com A=1/B=1 e duas com A=2/B=2.
- Contagem exata das threads e numeração dos itens de 1 até N.
- No deadlock, espera circular e esgotamento das duas classes de recurso.

Cada execução global tem limite de 15 segundos. Os testes do modo `ingenua` encerram o processo após 2 segundos e inspecionam o log para comprovar a espera circular. Um simples travamento não basta para aprovar.

Os logs, diagnósticos, resultados do verificador e o resumo JSON ficam em `resultados/`, ignorado pelo Git. As amostras de uma execução validada estão em `evidencias/`.

## GitHub

O workflow `.github/workflows/testes.yml` compila com avisos tratados como erros e roda a mesma rotina em Ubuntu a cada push ou pull request. Os logs ficam disponíveis como artefato na aba **Actions**.
