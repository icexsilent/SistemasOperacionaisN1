#ifndef ARGS_H
#define ARGS_H

/* Ordem de aquisição dos recursos. */
typedef enum {
    ORDEM_GLOBAL,   /* --ordem global   : ordem imposta, não trava            */
    ORDEM_INGENUA   /* --ordem ingenua  : sem coordenação, deve travar        */
} ordem_t;

typedef struct {
    int     produtores;
    int     consumidores;
    int     capacidade;    /* capacidade da estrutura compartilhada           */
    int     recursos_a;    /* instâncias disponíveis da classe A              */
    int     recursos_b;    /* instâncias disponíveis da classe B              */
    int     itens;         /* total de itens a produzir e consumir            */
    unsigned semente;      /* semente do gerador de números aleatórios        */
    ordem_t ordem;
} config_t;

/* Lê a linha de comando. Em caso de erro, imprime uso em stderr e encerra
   com código 1. Não há valores padrão: todos os parâmetros são obrigatórios. */
config_t args_parse(int argc, char **argv);

#endif
