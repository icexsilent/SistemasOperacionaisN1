/* Projeto 1 - Simulador multithread
 *
 * Este arquivo é um ponto de partida. Apague os comentários TODO conforme
 * for implementando, e organize o código em quantos arquivos .c fizerem
 * sentido: o Makefile compila tudo que estiver em src/.
 *
 * O que não pode mudar: o nome do executável, os parâmetros de linha de
 * comando (src/args.c) e o formato do log (src/log.h).
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "args.h"
#include "log.h"

/* Atraso, em microssegundos, entre a primeira e a segunda aquisição de
   recurso no modo --ordem ingenua.

   O atraso alarga a janela em que uma thread segura um recurso e ainda não
   pediu o outro, onde a espera circular já existe. Sem ele a janela dura
   nanossegundos e o programa termina normalmente na maioria das execuções. */
#define ATRASO_INGENUO_US 50000

int main(int argc, char **argv)
{
    config_t cfg = args_parse(argc, argv);

    log_iniciar();
    srand(cfg.semente);

    /* Mensagens de diagnóstico vão para stderr. O log de eventos, e apenas
       ele, vai para stdout. */
    fprintf(stderr, "config: %dp %dc cap=%d A=%d B=%d itens=%d ordem=%s\n",
            cfg.produtores, cfg.consumidores, cfg.capacidade,
            cfg.recursos_a, cfg.recursos_b, cfg.itens,
            cfg.ordem == ORDEM_GLOBAL ? "global" : "ingenua");

    /* TODO 1. Estrutura compartilhada de capacidade limitada.
       Produtor bloqueia quando cheia, consumidor bloqueia quando vazia.
       Loga BLOQUEIA_CHEIA e BLOQUEIA_VAZIA quando isso acontecer. */

    /* TODO 2. Duas classes de recurso escasso, com cfg.recursos_a e
       cfg.recursos_b instâncias. O consumidor precisa segurar uma instância
       de cada classe AO MESMO TEMPO para concluir um item.
       Loga SOLICITA antes de pedir, OBTEM ao conseguir, LIBERA ao devolver. */

    /* TODO 3. Threads produtoras e consumidoras.
       Cada thread loga CRIADA ao iniciar e ENCERRADA ao terminar.
       Produtor loga PRODUZIU com o id do item. Os ids formam uma única
       sequência (1, 2, 3, ...) compartilhada por todos os produtores.
       Consumidor loga CONSUMIU com o id do item, enquanto ainda segura as
       duas classes de recurso. */

    /* TODO 4. Modos de aquisição.
       ORDEM_GLOBAL:  ordem de aquisição imposta e igual para todos.
       ORDEM_INGENUA: sem coordenação, com usleep(ATRASO_INGENUO_US) entre a
                      primeira e a segunda aquisição. Deve travar. */

    /* TODO 5. Encerramento ordenado. Toda thread criada é joinada.
       Sem pthread_cancel, sem exit() no meio, sem sleep esperando dar tempo. */

    return 0;
}
