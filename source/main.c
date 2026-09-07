#include <stdio.h>
#include <string.h>
#include <errno.h>

int main(int argc, char *argv[])
  {
      if ( argc != 3 ) {
          fprintf(stderr, "Uso: %s <rate|edf> <arquivo>\n", argv[0]);
          return 1;
      }

      if ( strcmp(argv[1], "rate") != 0 && strcmp(argv[1], "edf") != 0 ) {
          fprintf(stderr, "Erro: algoritmo invalido: '%s' (use 'rate' ou 'edf').\n", argv[1]);
          return 1;
      }

      FILE *f = fopen(argv[2], "r");
      if (f == NULL) {
          fprintf(stderr, "Erro: nao foi possivel abrir '%s': %s\n", argv[2], strerror(errno));
          return 1;
      }

      fclose(f);

      return 0;
}
