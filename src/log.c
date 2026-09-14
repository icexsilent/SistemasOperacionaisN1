#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include "log.h"

static struct timespec t0;
static pthread_mutex_t mutex_log = PTHREAD_MUTEX_INITIALIZER;

void log_iniciar(void)
{
    clock_gettime(CLOCK_MONOTONIC, &t0);
}

static long agora_ms(void)
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (t.tv_sec - t0.tv_sec) * 1000L + (t.tv_nsec - t0.tv_nsec) / 1000000L;
}

/* Por que este mutex existe.
 *
 * Esta função é chamada por todas as threads do simulador, e stdout é um
 * recurso compartilhado como qualquer outro. A biblioteca padrão de C mantém
 * um lock interno por FILE, então um printf de uma chamada só costuma sair
 * inteiro mesmo sem proteção.
 *
 * Essa garantia é da implementação da biblioteca. Se a linha passar a ser
 * montada em dois printf, ou em partes, as linhas se intercalam. O mutex
 * acima mantém a saída correta por construção.
 */
void log_evento(const char *thread_id, const char *evento,
                const char *objeto, const char *detalhe)
{
    long t = agora_ms();

    pthread_mutex_lock(&mutex_log);
    printf("%ld;%s;%s;%s;%s\n",
           t,
           thread_id ? thread_id : "",
           evento,
           objeto  ? objeto  : "",
           detalhe ? detalhe : "");
    fflush(stdout);
    pthread_mutex_unlock(&mutex_log);
}
