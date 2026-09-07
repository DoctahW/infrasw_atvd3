#ifndef TAREFA_H
#define TAREFA_H
#include <stddef.h>
#include "erro.h"

#define MAX_TAREFAS 64
#define MAX_NOME    64

typedef struct {
    char nome[MAX_NOME];
    int  periodo;
    int  deadline;
    int  burst;
    int  indice;
    int  lost;
    int  complete;
    int  killed;
} TarefaRT;

int       tarefas_total(void);
TarefaRT *tarefa_em(int i);
TarefaRT *buscar_tarefa(const char *nome);
ErroParse adicionar_tarefa(const char *nome, int periodo, int deadline, int burst);

#endif
