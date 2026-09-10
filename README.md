# scheduler

Simulador de escalonamento de tempo real escrito em C para a disciplina
Infraestrutura de Software (00016) da CESAR School. O programa lê um conjunto de
tarefas periódicas de um arquivo e simula, instante a instante, a execução delas
em um único processador sob uma de duas políticas preemptivas: **rate-monotonic**
(`rate`, prioridade estática pelo menor período) ou **earliest-deadline-first**
(`edf`, prioridade dinâmica pelo deadline absoluto mais próximo). A saída é o
traço de execução mais a contagem de deadlines perdidos, execuções concluídas e
instâncias mortas pelo fim da simulação.

O núcleo é um simulador determinístico de eventos discretos com passo de 1 unidade
de tempo — não usa threads nem processos do sistema operacional; a preempção é
simulada. Só assim o traço fecha byte a byte com o esperado.

## Arquivos

```
.
├── source
│   ├── main.c
│   ├── parser.c
│   ├── parser.h
│   ├── tarefas.c
│   ├── tarefas.h
│   ├── erro.c
│   ├── erro.h
│   ├── sched.c
│   ├── sched.h
│   ├── saida.c
│   └── saida.h
├── tests
│   ├── f1_check.sh              smoke test do parser + validacao
│   ├── f5_check.sh              tabela de erros + casos de borda
│   ├── f6_check.sh              comparacao rate x edf no mesmo arquivo
│   ├── vetores
│   │   ├── voo.txt
│   │   ├── voo_esperado_rate.out
│   │   ├── rm_falha_edf_ok.txt
│   │   ├── rm_falha_edf_ok_esperado_rate.out
│   │   └── rm_falha_edf_ok_esperado_edf.out
│   └── testes_scheduler
├── objects
├── evidencias.log
├── Makefile
├── scheduler
└── README.md
```

Cada `.h` declara o que o `.c` de mesmo nome expõe. A tabela de tarefas é estática
e global (`source/tarefas.c`), então os scripts de teste rodam cada caso em uma
invocação separada do binário.

## Compilar

```
make
```

Gera o executável `scheduler` na raiz do projeto; os objetos ficam em `objects/`.
O login de entrega (`jems2`) é injetado pelo Makefile via `-DLOGIN`. Para limpar:

```
make clean        # remove objects/ e o binario
make distclean    # o de cima, mais os .out gerados
```

## Executar

```
./scheduler <rate|edf> <arquivo>
```

Exemplos:

```
./scheduler rate tests/vetores/voo.txt     # gera rate_jems2.out
./scheduler edf  tests/vetores/voo.txt     # gera edf_jems2.out
```

Na execução normal **nada é impresso em `stdout`**; o resultado vai para o arquivo
`rate_jems2.out` ou `edf_jems2.out`, na raiz. Erro (argumento errado, arquivo
inexistente, entrada malformada) vai para `stderr`, o programa sai com código
diferente de zero e **nenhum arquivo de saída é criado**.

### Formato de entrada

```
[TEMPO TOTAL]
[NOME] [PERIODO] [DEADLINE] [BURST]
[NOME] [PERIODO] [DEADLINE] [BURST]
...
```

- Primeira linha: um inteiro positivo, a duração da simulação.
- Cada linha seguinte é uma tarefa periódica. Todos os valores são inteiros
  positivos e vale `BURST <= DEADLINE <= PERIODO`.
- Toda tarefa chega pela primeira vez em `t = 0`; a instância `k` chega em
  `t = k * PERIODO` enquanto `k * PERIODO < TEMPO TOTAL`. Cada instância tem
  deadline absoluto `chegada + DEADLINE` e precisa de `BURST` unidades de CPU.
- Linhas em branco são ignoradas. Limites: até 64 tarefas, nome com até 63
  caracteres, linha cabendo no buffer de leitura de 512 bytes — estourar qualquer
  um é erro tratado (mensagem em `stderr`, exit ≠ 0, sem `.out`).

Exemplo (`tests/vetores/voo.txt`):

```
100
ATT 20 12 8
NAV 50 30 15
```

### Formato de saída

```
EXECUTION BY RATE
[ATT] for 8 units - F
[NAV] for 12 units - H
...
idle for 12 units

LOST DEADLINES
[ATT] 0
[NAV] 1

COMPLETE EXECUTION
[ATT] 5
[NAV] 1

KILLED
[ATT] 0
[NAV] 0
```

- Linha de execução: `[NOME] for N units - T`. A tag `T` é `F` (instância
  terminou), `H` (foi preemptada por uma tarefa de maior prioridade) ou `L`
  (perdeu o deadline enquanto executava). `K` marca a instância que ainda
  executava quando a simulação acabou.
- `idle for N units` é o processador ocioso, sem tag.
- Os três blocos de contadores listam as tarefas na ordem do arquivo de entrada.
  `LOST` conta deadlines perdidos; `COMPLETE`, instâncias concluídas; `KILLED`,
  instâncias cortadas pelo fim da simulação.

## Testar

```
make test
```

Compila e roda os três scripts de `tests/`, saindo com código diferente de zero
se algum falhar. Para rodar separadamente:

```
sh tests/f1_check.sh    # parser + validacao (F1)
sh tests/f5_check.sh    # tabela de erros da secao 10 + casos de borda (F5)
sh tests/f6_check.sh    # rate perde deadline e edf nao, no mesmo arquivo (F6)
```

`f6_check.sh` também compara `rate_jems2.out` / `edf_jems2.out` byte a byte com os
vetores de referência em `tests/vetores/` (`cmp -s`).

## Sistema operacional

Implementado e testado em Linux (CachyOS, kernel 7.2.3, x86_64) com GCC 16.2.1 e
padrão `c11`. Usa apenas a biblioteca padrão de C e uma função POSIX
(`_POSIX_C_SOURCE=200809L` para `strtok_r`), então deve compilar em qualquer
sistema POSIX com um compilador C11.
