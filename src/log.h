#ifndef LOG_H
#define LOG_H

/* Formato de saída, uma linha por evento, em stdout:

       tempo_ms;thread_id;EVENTO;objeto;detalhe

   Vocabulário fechado de eventos (qualquer outro invalida a entrega):

       CRIADA  PRODUZIU  BLOQUEIA_CHEIA  BLOQUEIA_VAZIA
       SOLICITA  OBTEM  LIBERA  CONSUMIU  ENCERRADA

   As classes de recurso se chamam "A" e "B" no campo objeto.

   Diagnóstico, depuração e erro vão para stderr, nunca para stdout. */

/* Marca o instante zero. Chamar uma única vez, no início da main. */
void log_iniciar(void);

/* Loga um evento. objeto e detalhe podem ser NULL (campo sai vazio).

   Exemplos:
       log_evento("prod-2", "CRIADA",   NULL, NULL);
       log_evento("prod-2", "PRODUZIU", "17", NULL);
       log_evento("cons-1", "SOLICITA", "A",  NULL);
       log_evento("cons-1", "CONSUMIU", "17", "tempo=42ms");
*/
void log_evento(const char *thread_id, const char *evento,
                const char *objeto, const char *detalhe);

#endif
