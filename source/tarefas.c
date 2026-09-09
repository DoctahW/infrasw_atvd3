#include "tarefas.h"
#include <string.h>
#include <stdbool.h>
#include <stddef.h>

static TarefaRT tabela[MAX_TAREFAS];
static int total = 0;

static bool copiar(char *destino, size_t tamanho, const char *origem) {
    if (origem == NULL || strlen(origem) >= tamanho) {
        return false;
    }
    strcpy(destino, origem);
    return true;
}

int tarefas_total(void) {
    return total;
}

TarefaRT *tarefa_em(int i) {
    if (i < 0 || i >= total) {
        return NULL;
    }
    return &tabela[i];
}

TarefaRT *buscar_tarefa(const char *nome) {
    for (int i = 0; i < total; i++) {
        if (strcmp(tabela[i].nome, nome) == 0) {
            return &tabela[i];
        }
    }
    return NULL;
}

ErroParse adicionar_tarefa(const char *nome, int periodo, int deadline, int burst) {
    if (total >= MAX_TAREFAS) {
        return ERR_TABELA_CHEIA;
    }
    if (buscar_tarefa(nome) != NULL) {
        return ERR_NOME_DUP;
    }
    if (!copiar(tabela[total].nome, sizeof tabela[total].nome, nome)) {
        return ERR_NOME_LONGO;
    }
    tabela[total].periodo = periodo;
    tabela[total].deadline = deadline;
    tabela[total].burst = burst;
    tabela[total].indice = total;
    total++;
    return OK;
}
