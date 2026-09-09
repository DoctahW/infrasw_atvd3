#include <string.h>
#include "sched.h"

typedef struct {
    int  restante;
    long deadline_abs;
    int  ativa;
} Instancia;

typedef struct {
    TarefaRT *dono;
    long inicio;
} Execucao;

static void fecha_segmento(Trace *trace, TarefaRT *dono, long inicio, long fim, char tag) {
    if (fim <= inicio) {
        return;
    }
    if (trace->num_seg >= MAX_SEGMENTOS) {
        return;
    }
    Segmento *s = &trace->segmentos[trace->num_seg++];
    s->t = dono;
    s->inicio = inicio;
    s->fim = fim;
    s->tag = tag;
}

static void admitir_chegadas(int n, Instancia *inst, long tick) {
    for (int i = 0; i < n; i++) {
        TarefaRT *tarefa = tarefa_em(i);
        if (tick % tarefa->periodo == 0) {
            inst[i].restante = tarefa->burst;
            inst[i].deadline_abs = tick + tarefa->deadline;
            inst[i].ativa = 1;
        }
    }
}

static void expirar_deadlines(int n, Instancia *inst, long tick, Trace *trace, Execucao *ex) {
    for (int i = 0; i < n; i++) {
        Instancia *instancia = &inst[i];
        if (!instancia->ativa || instancia->restante == 0 || instancia->deadline_abs != tick) {
            continue;
        }

        TarefaRT *tarefa = tarefa_em(i);
        if (ex->dono == tarefa) {
            fecha_segmento(trace, ex->dono, ex->inicio, tick, 'L');
            ex->dono = NULL;
            ex->inicio = tick;
        }
        tarefa->lost++;
        instancia->ativa = 0;
    }
}

static void finalizar_se_concluida(Instancia *inst, long tick, Trace *trace, Execucao *ex) {
    if (ex->dono != NULL && inst[ex->dono->indice].restante == 0) {
        fecha_segmento(trace, ex->dono, ex->inicio, tick, 'F');
        ex->dono->complete++;
        inst[ex->dono->indice].ativa = 0;
        ex->dono = NULL;
        ex->inicio = tick;
    }
}

static long chave_prioridade(Politica pol, const Instancia *inst, int i) {
    if (pol == POL_RATE) {
        return tarefa_em(i)->periodo;
    }
    return inst[i].deadline_abs;
}

static int mais_prioritaria(Politica pol, const Instancia *inst, int i, int j) {
    long ki = chave_prioridade(pol, inst, i);
    long kj = chave_prioridade(pol, inst, j);
    if (ki != kj) {
        return (ki < kj) ? -1 : 1;
    }
    return tarefa_em(i)->indice - tarefa_em(j)->indice;
}

static TarefaRT *selecionar_proxima(Politica pol, int n, Instancia *inst) {
    int melhor = -1;
    for (int i = 0; i < n; i++) {
        if (!inst[i].ativa || inst[i].restante <= 0) {
            continue;
        }
        if (melhor < 0 || mais_prioritaria(pol, inst, i, melhor) < 0) {
            melhor = i;
        }
    }
    return (melhor < 0) ? NULL : tarefa_em(melhor);
}

static void trocar_se_necessario(TarefaRT *selecionada, long tick, Trace *trace, Execucao *ex) {
    if (selecionada == ex->dono) {
        return;
    }
    char tag = (ex->dono != NULL) ? 'H' : 0;
    fecha_segmento(trace, ex->dono, ex->inicio, tick, tag);
    ex->dono = selecionada;
    ex->inicio = tick;
}

static void contabilizar_perdas_finais(int n, Instancia *inst) {
    for (int i = 0; i < n; i++) {
        if (inst[i].ativa && inst[i].restante > 0) {
            tarefa_em(i)->killed++;
        }
    }
}

int simula(Politica pol, long total, Trace *trace) {
    int n = tarefas_total();
    static Instancia inst[MAX_TAREFAS];
    memset(inst, 0, sizeof inst);

    trace->num_seg = 0;
    Execucao ex = { .dono = NULL, .inicio = 0 };

    for (long tick = 0; tick < total; tick++) {
        finalizar_se_concluida(inst, tick, trace, &ex);
        expirar_deadlines(n, inst, tick, trace, &ex);
        admitir_chegadas(n, inst, tick);

        TarefaRT *selecionada = selecionar_proxima(pol, n, inst);
        trocar_se_necessario(selecionada, tick, trace, &ex);

        if (selecionada != NULL) {
            inst[selecionada->indice].restante--;
        }
    }

    fecha_segmento(trace, ex.dono, ex.inicio, total, ex.dono ? 'K' : 0);
    contabilizar_perdas_finais(n, inst);

    return (trace->num_seg >= MAX_SEGMENTOS) ? -1 : 0;
}
