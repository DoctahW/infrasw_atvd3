#ifndef ERRO_H
#define ERRO_H

// mesmo padrão de erro do ProcessFlow, mas como tem mais opções separei e outro arquivo
typedef enum ErroParse {
    OK=0,
    ERR_TOTAL,
    ERR_SEM_TAREFAS,
    ERR_LINHA_LONGA,
    ERR_CAMPOS,
    ERR_NAO_NUMERICO,
    ERR_NAO_POSITIVO,
    ERR_OVERFLOW,
    ERR_D_MAIOR_P,
    ERR_C_MAIOR_D,
    ERR_NOME_LONGO,
    ERR_NOME_DUP,
    ERR_TABELA_CHEIA
} ErroParse;

const char *erro_msg(ErroParse e);

#endif
