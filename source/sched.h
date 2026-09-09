#ifndef SCHED_H
#define SCHED_H

#include "tarefas.h"

typedef enum {
    POL_RATE,
    POL_EDF
} Politica;

#define MAX_SEGMENTOS 65536

typedef struct {
    TarefaRT *t;
    long      inicio;
    long      fim;
    char      tag;
} Segmento;

typedef struct {
    Segmento segmentos[MAX_SEGMENTOS];
    int      num_seg;
} Trace;


int simula(Politica pol, long total, Trace *trace);

#endif
