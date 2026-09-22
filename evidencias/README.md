# Evidências de execução

As duas amostras foram produzidas pelo executável compilado no Windows com MSYS2 UCRT64 e aprovadas pelo verificador original.

- `global.log`: 3 produtores, 4 consumidores, capacidade 5, A=2, B=2, 30 itens, semente 1, modo global.
- `deadlock.log`: mesma configuração, modo ingenua; o processo foi encerrado externamente após 2 segundos.
- `global.verificacao.txt` e `deadlock.verificacao.txt`: saídas reais de `verificar.py`.

Para conferir as amostras:

```sh
python3 verificar.py evidencias/global.log --itens 30 --recursos-a 2 --recursos-b 2
python3 verificar.py evidencias/deadlock.log --deadlock
```

A rotina completa aprovou 12 de 12 cenários. Execute `make test` para gerar novos resultados.
