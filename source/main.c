#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "parser.h"
#include "erro.h"
#include "sched.h"
#include "saida.h"

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <rate|edf> <arquivo>\n", argv[0]);
        return 1;
    }

    Politica pol;
    if (strcmp(argv[1], "rate") == 0) {
        pol = POL_RATE;
    } else if (strcmp(argv[1], "edf") == 0) {
        pol = POL_EDF;
    } else {
        fprintf(stderr, "Erro: algoritmo invalido: '%s' (use 'rate' ou 'edf').\n", argv[1]);
        return 1;
    }

    FILE *f = fopen(argv[2], "r");
    if (f == NULL) {
        fprintf(stderr, "Erro: nao foi possivel abrir '%s': %s\n", argv[2], strerror(errno));
        return 1;
    }

    int total;
    char erro[256];
    ErroParse e = le_entrada(f, &total, erro, sizeof erro);

    fclose(f);

    if (e != OK) {
        fprintf(stderr, "Erro: %s\n", erro);
        return 1;
    }

    static Trace trace;
    if (simula(pol, total, &trace) != 0) {
        fprintf(stderr, "Erro: a simulacao excedeu o limite de segmentos do trace.\n");
        return 1;
    }

    char caminho[64];
    snprintf(caminho, sizeof caminho, "%s_%s.out",
             pol == POL_RATE ? "rate" : "edf", LOGIN);

    if (escreve_saida(caminho, pol, &trace) != 0) {
        fprintf(stderr, "Erro: nao foi possivel escrever '%s'.\n", caminho);
        return 1;
    }

    return 0;
}
