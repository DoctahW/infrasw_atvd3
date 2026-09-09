#include <stdio.h>
#include "saida.h"
#include "tarefas.h"

int escreve_saida(const char *caminho, Politica pol, const Trace *trace) {
    FILE *f = fopen(caminho, "w");
    if (f == NULL){
        return -1;
    }

    fprintf(f, "EXECUTION BY %s\n", pol == POL_RATE ? "RATE" : "EDF");

    for (int i = 0; i < trace->num_seg; i++) {
        const Segmento *s = &trace->segmentos[i];
        long dur = s->fim - s->inicio;

        if (s->t == NULL) {
            fprintf(f, "idle for %ld units\n", dur);
        } else if (s->tag == 0) {
            fprintf(f, "[%s] for %ld units\n", s->t->nome, dur);
        } else {
            fprintf(f, "[%s] for %ld units - %c\n", s->t->nome, dur, s->tag);
        }
    }

    fprintf(f, "\nLOST DEADLINES\n");
    for (int i = 0; i < tarefas_total(); i++) {
        const TarefaRT *t = tarefa_em(i);
        fprintf(f, "[%s] %d\n", t->nome, t->lost);
    }

    fprintf(f, "\nCOMPLETE EXECUTION\n");
    for (int i = 0; i < tarefas_total(); i++) {
        const TarefaRT *t = tarefa_em(i);
        fprintf(f, "[%s] %d\n", t->nome, t->complete);
    }

    fprintf(f, "\nKILLED\n");
    for (int i = 0; i < tarefas_total(); i++) {
        const TarefaRT *t = tarefa_em(i);
        fprintf(f, "[%s] %d\n", t->nome, t->killed);
    }

    if (ferror(f)){
        fclose(f);
        return -1;
    }
    if (fclose(f) != 0){
        return -1;
    }
    return 0;
}
