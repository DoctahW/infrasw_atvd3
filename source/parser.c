#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include "parser.h"
#include "tarefas.h"

static void descartar_resto(FILE *arquivo) {
    int c;
    do {
        c = fgetc(arquivo);
    } while (c != '\n' && c != EOF);
}

static ErroParse para_pos_int(const char *argv, int *saida){
    char *fim;
    errno = 0;
    long valor = strtol(argv, &fim, 10);

    if (fim == argv){
        return ERR_NAO_NUMERICO;
    }

    if (*fim != '\0') {
        return ERR_NAO_NUMERICO;
    }

    if (errno == ERANGE || valor > INT_MAX){
        return ERR_OVERFLOW;
    }

    if (valor <= 0){
        return ERR_NAO_POSITIVO;
    }

    *saida = (int)valor;
    return OK;
}

int dividir_palavras(char *linha, char **palavras, int quant) {
    if (quant <= 0) {
        return 0;
    }
    char *saveptr;
    char *token = strtok_r(linha, " \t\n", &saveptr);
    int n = 0;

    while (token != NULL && n < quant - 1) {
        palavras[n] = token;
        n++;
        token = strtok_r(NULL, " \t\n", &saveptr);
    }

    palavras[n] = NULL;
    return n;
}

ErroParse le_entrada(FILE *f, int *total_out, char *erro, size_t erro_sz) {
    char linha[512];
    char *campos[8];
    int nlinha;
    int total = 0;

    if (fgets(linha, sizeof linha, f) == NULL) {
        snprintf(erro, erro_sz, "linha 1: %s", erro_msg(ERR_TOTAL));
        return ERR_TOTAL;
    }
    if (strchr(linha, '\n') == NULL && !feof(f)) {
        descartar_resto(f);
        snprintf(erro, erro_sz, "linha 1: %s", erro_msg(ERR_LINHA_LONGA));
        return ERR_LINHA_LONGA;
    }
    if (dividir_palavras(linha, campos, 8) != 1) {
        snprintf(erro, erro_sz, "linha 1: %s", erro_msg(ERR_TOTAL));
        return ERR_TOTAL;
    }
    {
        ErroParse e = para_pos_int(campos[0], &total);
        if (e != OK) {
            snprintf(erro, erro_sz, "linha 1: tempo total '%s': %s", campos[0], erro_msg(e));
            return e;
        }
    }

    nlinha = 1;
    while (fgets(linha, sizeof linha, f)) {
        nlinha++;

        if (strchr(linha, '\n') == NULL && !feof(f)) {
            descartar_resto(f);
            snprintf(erro, erro_sz, "linha %d: %s", nlinha, erro_msg(ERR_LINHA_LONGA));
            return ERR_LINHA_LONGA;
        }

        int n_campos = dividir_palavras(linha, campos, 8);

        if (n_campos == 0) {
            continue;
        }

        if (n_campos != 4) {
            snprintf(erro, erro_sz, "linha %d: %s", nlinha, erro_msg(ERR_CAMPOS));
            return ERR_CAMPOS;
        }

        int periodo, deadline, burst;
        ErroParse e;

        e = para_pos_int(campos[1], &periodo);
        if (e != OK) {
            snprintf(erro, erro_sz, "linha %d: periodo '%s': %s", nlinha, campos[1], erro_msg(e));
            return e;
        }

        e = para_pos_int(campos[2], &deadline);
        if (e != OK) {
            snprintf(erro, erro_sz, "linha %d: deadline '%s': %s", nlinha, campos[2], erro_msg(e));
            return e;
        }

        e = para_pos_int(campos[3], &burst);
        if (e != OK) {
            snprintf(erro, erro_sz, "linha %d: burst '%s': %s", nlinha, campos[3], erro_msg(e));
            return e;
        }

        if (deadline > periodo) {
            snprintf(erro, erro_sz, "linha %d: tarefa '%s': %s",
                     nlinha, campos[0], erro_msg(ERR_D_MAIOR_P));
            return ERR_D_MAIOR_P;
        }

        if (burst > deadline) {
            snprintf(erro, erro_sz, "linha %d: tarefa '%s': %s",
                     nlinha, campos[0], erro_msg(ERR_C_MAIOR_D));
            return ERR_C_MAIOR_D;
        }

        e = adicionar_tarefa(campos[0], periodo, deadline, burst);
        if (e != OK) {
            snprintf(erro, erro_sz, "linha %d: tarefa '%s': %s", nlinha, campos[0], erro_msg(e));
            return e;
        }
    }

    if (tarefas_total() == 0) {
        snprintf(erro, erro_sz, "%s", erro_msg(ERR_SEM_TAREFAS));
        return ERR_SEM_TAREFAS;
    }

    *total_out = total;
    return OK;
}
