#include "erro.h"

// igual o mensagem_status da atvd1
const char *erro_msg(ErroParse e){
    switch (e) {
        case OK:               return "sem erro";
        case ERR_TOTAL:        return "deve conter so o tempo total de simulacao (um inteiro positivo)";
        case ERR_SEM_TAREFAS:  return "o arquivo nao define nenhuma tarefa";
        case ERR_LINHA_LONGA:  return "linha comprida demais para o buffer de leitura";
        case ERR_CAMPOS:       return "linha de tarefa fora do formato 'NOME PERIODO DEADLINE BURST'";
        case ERR_NAO_NUMERICO: return "valor nao e um inteiro valido";
        case ERR_NAO_POSITIVO: return "valor deve ser um inteiro maior que zero";
        case ERR_OVERFLOW:     return "valor grande demais";
        case ERR_D_MAIOR_P:    return "deadline maior que o periodo";
        case ERR_C_MAIOR_D:    return "burst maior que o deadline";
        case ERR_NOME_LONGO:   return "nome da tarefa longo demais";
        case ERR_NOME_DUP:     return "ja existe uma tarefa com esse nome";
        case ERR_TABELA_CHEIA: return "tarefas demais para a tabela";
    }
    return "motivo desconhecido";
}
