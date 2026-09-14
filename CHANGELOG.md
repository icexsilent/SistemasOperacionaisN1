# Changelog

Mudanças na especificação do Projeto 1.

Sempre que houver uma entrada nova aqui, rode `git pull` no seu clone. Se a
mudança afetar o que já foi implementado, o aviso vem junto na entrada.

## v1 (2026-09-02)

Versão inicial

- `README.md`: contextualização, objetivos de aprendizagem, instruções de
  compilação e execução, rubrica de avaliação e regras de trabalho em equipe.
- `RELATORIO.md`: roteiro das cinco seções exigidas.
- `USO_DE_IA.md`: declaração de uso de IA.
- `Makefile`: compila todo arquivo `.c` em `src/`.
- `src/args.c`, `src/args.h`: leitura da linha de comando. Prontos.
- `src/log.c`, `src/log.h`: log de eventos no formato
  `tempo_ms;thread_id;EVENTO;objeto;detalhe`. Prontos.
- `src/main.c`: esqueleto com os cinco `TODO`.
- `verificar.py`: script de verificação dos invariantes do log, o mesmo usado
  na correção. Validado contra uma solução de referência: aprova o log de uma
  implementação correta em 40 sementes, detecta a espera circular do modo
  `ingenua` e reprova log truncado, evento fora do vocabulário, thread sem
  `ENCERRADA` (ou com mais de uma), recurso não liberado, `CONSUMIU` sem as
  duas classes, `OBTEM` sem `SOLICITA`, `CONSUMIU` antes do `PRODUZIU` do
  mesmo item e classe de recurso com nome diferente de `A` e `B`.
