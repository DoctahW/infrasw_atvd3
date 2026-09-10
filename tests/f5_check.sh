#!/bin/sh
set -u

cd "$(dirname "$0")/.." || exit 1
make || { echo "make FALHOU"; exit 1; }

raiz=$(pwd)
tmp=$(mktemp -d)
trap 'chmod -R u+rwx "$tmp" 2>/dev/null; rm -rf "$tmp"' EXIT
ok=0
falha=0

limpar() { rm -f rate_jems2.out edf_jems2.out; }

# cenario de erro: exit != 0, stderr nao vazio, stdout vazio, nenhum .out criado.
# $1 = descricao; o resto vai direto pro scheduler (assim cobre argv e arquivo).
esperar_erro() {
    desc=$1
    shift
    limpar
    saida=$(./scheduler "$@" 2>"$tmp/err"); rc=$?
    if [ "$rc" -ne 0 ] && [ -s "$tmp/err" ] && [ -z "$saida" ] \
       && [ ! -e rate_jems2.out ] && [ ! -e edf_jems2.out ]; then
        echo "ok     $desc  ->  $(head -1 "$tmp/err")"; ok=$((ok + 1))
    else
        echo "FALHA  $desc  (rc=$rc, stdout='$saida')"; falha=$((falha + 1))
    fi
}

# cenario valido: exit 0, stdout vazio, nada em stderr, o .out da politica criado.
# agora o binario esta completo, entao a entrada boa TEM que gerar arquivo.
esperar_ok() {
    pol=$1; arq=$2; desc=$3
    limpar
    saida=$(./scheduler "$pol" "$arq" 2>"$tmp/err"); rc=$?
    if [ "$rc" -eq 0 ] && [ -z "$saida" ] && [ ! -s "$tmp/err" ] && [ -s "${pol}_jems2.out" ]; then
        echo "ok     $desc"; ok=$((ok + 1))
    else
        echo "FALHA  $desc  (rc=$rc, stdout='$saida', err='$(head -1 "$tmp/err")')"
        falha=$((falha + 1))
    fi
}

echo "-- tabela de erros (secao 10 do PLANO) --"
esperar_erro "1  sem argumentos"
esperar_erro "2  poucos argumentos"      rate
esperar_erro "3  argumentos demais"      rate tests/vetores/voo.txt extra
esperar_erro "4  algoritmo != rate/edf"  foo tests/vetores/voo.txt
esperar_erro "5  arquivo inexistente"    rate "$tmp/naoexiste.txt"

printf 'abc\n'                > "$tmp/l1nn";  esperar_erro "7  linha 1 nao numerica" rate "$tmp/l1nn"
printf '0\nATT 20 12 8\n'     > "$tmp/l1np";  esperar_erro "7  linha 1 <= 0"         rate "$tmp/l1np"
printf '100\nATT 20 12\n'     > "$tmp/campo"; esperar_erro "8  campo faltando"       rate "$tmp/campo"
printf '100\nATT 20 doze 8\n' > "$tmp/nnum";  esperar_erro "9  valor nao numerico"   rate "$tmp/nnum"
printf '100\nATT 20 0 8\n'    > "$tmp/npos";  esperar_erro "10 valor nao positivo"   rate "$tmp/npos"
printf '100\nATT 20 25 8\n'   > "$tmp/dp";    esperar_erro "11 D > P"                rate "$tmp/dp"
printf '100\nATT 20 12 15\n'  > "$tmp/cd";    esperar_erro "12 C > D"                rate "$tmp/cd"

echo
echo "-- limites da tabela estatica e do buffer (F5) --"

# 6  entrada sem permissao de leitura -- root ignora modo, entao pula
if [ "$(id -u)" -eq 0 ]; then
    echo "pula   6  entrada ilegivel (rodando como root)"
else
    printf '100\nATT 20 12 8\n' > "$tmp/semperm"; chmod 000 "$tmp/semperm"
    esperar_erro "6  entrada ilegivel" rate "$tmp/semperm"
    chmod 644 "$tmp/semperm"
fi

# MAX_TAREFAS e 64: 65 tarefas tem que estourar com erro tratado, sem crash
{ echo 1000; i=0; while [ "$i" -lt 65 ]; do echo "T$i 100 100 1"; i=$((i + 1)); done; } > "$tmp/cheia"
esperar_erro "tabela cheia (65 > MAX_TAREFAS)" rate "$tmp/cheia"

# MAX_NOME e 64: nome com 80 caracteres nao pode entrar
nome=; i=0; while [ "$i" -lt 80 ]; do nome="${nome}N"; i=$((i + 1)); done
{ echo 100; echo "$nome 20 12 8"; } > "$tmp/nomao"
esperar_erro "nome da tarefa longo demais" rate "$tmp/nomao"

# buffer de leitura e 512: linha maior que isso, sem \n, cai no ERR_LINHA_LONGA
{ printf '100\n'; printf 'X 20 12 8 '; head -c 600 /dev/zero | tr '\0' '#'; printf '\n'; } > "$tmp/longa"
esperar_erro "linha comprida demais para o buffer" rate "$tmp/longa"

echo
echo "-- casos de borda que TEM que rodar (exit 0, .out gerado, stdout vazio) --"
esperar_ok rate tests/testes_scheduler/borda_c_igual_d_igual_p.txt "14 uma tarefa com C == D == P"
esperar_ok rate tests/testes_scheduler/borda_c_igual_d.txt         "C == D, uma tarefa"
esperar_ok rate tests/testes_scheduler/borda_d_igual_p.txt         "D == P sob RATE (perde deadline mas roda)"
esperar_ok edf  tests/testes_scheduler/borda_d_igual_p.txt         "D == P sob EDF"

echo
echo "-- 13 diretorio de saida sem permissao de escrita --"
if [ "$(id -u)" -eq 0 ]; then
    echo "pula   13 saida sem permissao (rodando como root)"
else
    mkdir "$tmp/so_leitura"; chmod 555 "$tmp/so_leitura"
    saida=$(cd "$tmp/so_leitura" && "$raiz/scheduler" rate "$raiz/tests/vetores/voo.txt" 2>"$tmp/err"); rc=$?
    if [ "$rc" -ne 0 ] && [ -s "$tmp/err" ] && [ -z "$saida" ] \
       && [ ! -e "$tmp/so_leitura/rate_jems2.out" ]; then
        echo "ok     13 erro ao criar .out, sem crash  ->  $(head -1 "$tmp/err")"; ok=$((ok + 1))
    else
        echo "FALHA  13 (rc=$rc, stdout='$saida')"; falha=$((falha + 1))
    fi
    chmod 755 "$tmp/so_leitura"
fi

limpar
echo
echo "resultado: $ok ok, $falha falha"
[ "$falha" -eq 0 ]
