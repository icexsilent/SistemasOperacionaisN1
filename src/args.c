/* Leitura da linha de comando. Este arquivo está pronto e não precisa ser
   alterado. Se você alterar os nomes dos parâmetros, a correção automática
   falha. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "args.h"

static void uso(const char *prog)
{
    fprintf(stderr,
        "uso: %s --produtores N --consumidores N --capacidade N \\\n"
        "        --recursos-a N --recursos-b N --itens N \\\n"
        "        --semente N --ordem [global|ingenua]\n", prog);
    exit(1);
}

static int inteiro(const char *s, const char *flag, const char *prog)
{
    char *fim;
    long v = strtol(s, &fim, 10);
    if (*fim != '\0' || v < 0 || v > 100000) {
        fprintf(stderr, "valor inválido para %s: %s\n", flag, s);
        uso(prog);
    }
    return (int)v;
}

config_t args_parse(int argc, char **argv)
{
    config_t c;
    int vistos = 0;

    memset(&c, 0, sizeof c);

    for (int i = 1; i < argc; i++) {
        if (i + 1 >= argc) uso(argv[0]);
        const char *f = argv[i];
        const char *v = argv[++i];

        if      (!strcmp(f, "--produtores"))   { c.produtores   = inteiro(v, f, argv[0]); vistos |= 1;   }
        else if (!strcmp(f, "--consumidores")) { c.consumidores = inteiro(v, f, argv[0]); vistos |= 2;   }
        else if (!strcmp(f, "--capacidade"))   { c.capacidade   = inteiro(v, f, argv[0]); vistos |= 4;   }
        else if (!strcmp(f, "--recursos-a"))   { c.recursos_a   = inteiro(v, f, argv[0]); vistos |= 8;   }
        else if (!strcmp(f, "--recursos-b"))   { c.recursos_b   = inteiro(v, f, argv[0]); vistos |= 16;  }
        else if (!strcmp(f, "--itens"))        { c.itens        = inteiro(v, f, argv[0]); vistos |= 32;  }
        else if (!strcmp(f, "--semente"))      { c.semente = (unsigned)inteiro(v, f, argv[0]); vistos |= 64; }
        else if (!strcmp(f, "--ordem")) {
            if      (!strcmp(v, "global"))  c.ordem = ORDEM_GLOBAL;
            else if (!strcmp(v, "ingenua")) c.ordem = ORDEM_INGENUA;
            else uso(argv[0]);
            vistos |= 128;
        }
        else uso(argv[0]);
    }

    if (vistos != 255) {
        fprintf(stderr, "faltam parâmetros obrigatórios\n");
        uso(argv[0]);
    }
    if (c.produtores < 1 || c.consumidores < 1 || c.capacidade < 1 ||
        c.recursos_a < 1 || c.recursos_b < 1 || c.itens < 1) {
        fprintf(stderr, "todos os parâmetros numéricos devem ser >= 1\n");
        exit(1);
    }
    return c;
}
