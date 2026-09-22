/* Projeto 1 - simulador de produtores, consumidores e recursos escassos. */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "args.h"
#include "log.h"

#define ATRASO_INGENUO_US 50000

typedef struct {
    config_t cfg;
    /* A fila, a numeracao e o fim da producao usam o mesmo mutex. */
    pthread_mutex_t fila_mutex;
    pthread_cond_t nao_cheia;
    pthread_cond_t nao_vazia;
    int *fila;
    int inicio, fim, quantidade, proximo_item, produtores_ativos;

    pthread_mutex_t recursos_mutex;
    pthread_cond_t recurso_livre[2];
    int disponiveis[2];
} simulador_t;

typedef struct {
    simulador_t *sim;
    int indice;
    char nome[32];
} thread_t;

static void falha(const char *mensagem)
{
    perror(mensagem);
    exit(EXIT_FAILURE);
}

static void obter(simulador_t *sim, const char *nome, int classe)
{
    const char *objeto = classe == 0 ? "A" : "B";
    pthread_mutex_lock(&sim->recursos_mutex);
    log_evento(nome, "SOLICITA", objeto, NULL);
    while (sim->disponiveis[classe] == 0)
        pthread_cond_wait(&sim->recurso_livre[classe], &sim->recursos_mutex);
    --sim->disponiveis[classe];
    /* O evento acompanha a alteracao do contador sob o mesmo mutex. */
    log_evento(nome, "OBTEM", objeto, NULL);
    pthread_mutex_unlock(&sim->recursos_mutex);
}

static void liberar(simulador_t *sim, const char *nome, int classe)
{
    const char *objeto = classe == 0 ? "A" : "B";
    pthread_mutex_lock(&sim->recursos_mutex);
    ++sim->disponiveis[classe];
    log_evento(nome, "LIBERA", objeto, NULL);
    pthread_cond_signal(&sim->recurso_livre[classe]);
    pthread_mutex_unlock(&sim->recursos_mutex);
}

static void *produtor(void *arg)
{
    thread_t *thread = arg;
    simulador_t *sim = thread->sim;
    log_evento(thread->nome, "CRIADA", NULL, NULL);

    for (;;) {
        pthread_mutex_lock(&sim->fila_mutex);
        while (sim->quantidade == sim->cfg.capacidade &&
               sim->proximo_item <= sim->cfg.itens) {
            log_evento(thread->nome, "BLOQUEIA_CHEIA", NULL, NULL);
            pthread_cond_wait(&sim->nao_cheia, &sim->fila_mutex);
        }
        if (sim->proximo_item > sim->cfg.itens) {
            pthread_mutex_unlock(&sim->fila_mutex);
            break;
        }

        int item = sim->proximo_item++;
        sim->fila[sim->fim] = item;
        sim->fim = (sim->fim + 1) % sim->cfg.capacidade;
        ++sim->quantidade;
        char numero[32];
        snprintf(numero, sizeof numero, "%d", item);
        log_evento(thread->nome, "PRODUZIU", numero, NULL);
        pthread_cond_signal(&sim->nao_vazia);
        if (sim->proximo_item > sim->cfg.itens)
            pthread_cond_broadcast(&sim->nao_cheia);
        pthread_mutex_unlock(&sim->fila_mutex);
    }

    pthread_mutex_lock(&sim->fila_mutex);
    --sim->produtores_ativos;
    pthread_cond_broadcast(&sim->nao_vazia);
    pthread_mutex_unlock(&sim->fila_mutex);
    log_evento(thread->nome, "ENCERRADA", NULL, NULL);
    return NULL;
}

static void *consumidor(void *arg)
{
    thread_t *thread = arg;
    simulador_t *sim = thread->sim;
    log_evento(thread->nome, "CRIADA", NULL, NULL);

    for (;;) {
        pthread_mutex_lock(&sim->fila_mutex);
        while (sim->quantidade == 0 && sim->produtores_ativos > 0) {
            log_evento(thread->nome, "BLOQUEIA_VAZIA", NULL, NULL);
            pthread_cond_wait(&sim->nao_vazia, &sim->fila_mutex);
        }
        if (sim->quantidade == 0) {
            pthread_mutex_unlock(&sim->fila_mutex);
            break;
        }
        int item = sim->fila[sim->inicio];
        sim->inicio = (sim->inicio + 1) % sim->cfg.capacidade;
        --sim->quantidade;
        pthread_cond_signal(&sim->nao_cheia);
        pthread_mutex_unlock(&sim->fila_mutex);

        int primeiro = 0;
        if (sim->cfg.ordem == ORDEM_INGENUA) {
            /* No exemplo A=2, B=2, dois seguram A e dois seguram B. */
            primeiro = thread->indice >= sim->cfg.recursos_a ? 1 : 0;
        }
        int segundo = 1 - primeiro;
        obter(sim, thread->nome, primeiro);
        if (sim->cfg.ordem == ORDEM_INGENUA)
            usleep(ATRASO_INGENUO_US);
        obter(sim, thread->nome, segundo);

        char numero[32];
        snprintf(numero, sizeof numero, "%d", item);
        log_evento(thread->nome, "CONSUMIU", numero, NULL);
        liberar(sim, thread->nome, segundo);
        liberar(sim, thread->nome, primeiro);
    }

    log_evento(thread->nome, "ENCERRADA", NULL, NULL);
    return NULL;
}

int main(int argc, char **argv)
{
    config_t cfg = args_parse(argc, argv);
    simulador_t sim = {0};
    sim.cfg = cfg;
    sim.fila = malloc((size_t)cfg.capacidade * sizeof *sim.fila);
    pthread_t *produtores = calloc((size_t)cfg.produtores, sizeof *produtores);
    pthread_t *consumidores = calloc((size_t)cfg.consumidores, sizeof *consumidores);
    thread_t *prod_args = calloc((size_t)cfg.produtores, sizeof *prod_args);
    thread_t *cons_args = calloc((size_t)cfg.consumidores, sizeof *cons_args);
    if (!sim.fila || !produtores || !consumidores || !prod_args || !cons_args)
        falha("alocacao de memoria");

    sim.proximo_item = 1;
    sim.produtores_ativos = cfg.produtores;
    sim.disponiveis[0] = cfg.recursos_a;
    sim.disponiveis[1] = cfg.recursos_b;
    pthread_mutex_init(&sim.fila_mutex, NULL);
    pthread_cond_init(&sim.nao_cheia, NULL);
    pthread_cond_init(&sim.nao_vazia, NULL);
    pthread_mutex_init(&sim.recursos_mutex, NULL);
    pthread_cond_init(&sim.recurso_livre[0], NULL);
    pthread_cond_init(&sim.recurso_livre[1], NULL);

    log_iniciar();
    fprintf(stderr, "config: %dp %dc cap=%d A=%d B=%d itens=%d ordem=%s\n",
            cfg.produtores, cfg.consumidores, cfg.capacidade,
            cfg.recursos_a, cfg.recursos_b, cfg.itens,
            cfg.ordem == ORDEM_GLOBAL ? "global" : "ingenua");

    for (int i = 0; i < cfg.produtores; ++i) {
        prod_args[i].sim = &sim;
        prod_args[i].indice = i;
        snprintf(prod_args[i].nome, sizeof prod_args[i].nome, "prod-%d", i + 1);
        if (pthread_create(&produtores[i], NULL, produtor, &prod_args[i]) != 0)
            falha("pthread_create produtor");
    }
    for (int i = 0; i < cfg.consumidores; ++i) {
        cons_args[i].sim = &sim;
        cons_args[i].indice = i;
        snprintf(cons_args[i].nome, sizeof cons_args[i].nome, "cons-%d", i + 1);
        if (pthread_create(&consumidores[i], NULL, consumidor, &cons_args[i]) != 0)
            falha("pthread_create consumidor");
    }

    for (int i = 0; i < cfg.produtores; ++i)
        pthread_join(produtores[i], NULL);
    for (int i = 0; i < cfg.consumidores; ++i)
        pthread_join(consumidores[i], NULL);

    pthread_cond_destroy(&sim.recurso_livre[1]);
    pthread_cond_destroy(&sim.recurso_livre[0]);
    pthread_mutex_destroy(&sim.recursos_mutex);
    pthread_cond_destroy(&sim.nao_vazia);
    pthread_cond_destroy(&sim.nao_cheia);
    pthread_mutex_destroy(&sim.fila_mutex);
    free(cons_args);
    free(prod_args);
    free(consumidores);
    free(produtores);
    free(sim.fila);
    return 0;
}
