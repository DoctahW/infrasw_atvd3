#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include "erro.h"

int dividir_palavras(char *linha, char **palavras, int quant);
ErroParse le_entrada(FILE *f, int *total_out, char *erro, size_t erro_sz);
#endif
