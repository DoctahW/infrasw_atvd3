#!/bin/sh
# tests/f1_check.sh -- checagem rapida da F1 (parser + validacao).
# Gerado com auxilio de IA (ver secao 7 do relatorio, divida D.9).
# Nao substitui o run_tests.sh completo da F7; e so um smoke test.
set -u

cd "$(dirname "$0")/.." || exit 1
make || { echo "make FALHOU"; exit 1; }

tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT
ok=0
falha=0

limpar() { rm -f rate_jems2.out edf_jems2.out; }

# entrada valida: exit 0, stdout vazio, nenhum .out criado
esperar_ok() {
    limpar
    saida=$(./scheduler rate "$1" 2>/dev/null); rc=$?
    if [ "$rc" -eq 0 ] && [ -z "$saida" ] && [ ! -e rate_jems2.out ]; then
        echo "ok     $2"; ok=$((ok + 1))
    else
        echo "FALHA  $2  (rc=$rc, stdout='$saida')"; falha=$((falha + 1))
    fi
}

# entrada invalida: exit != 0, stderr nao vazio, stdout vazio, nenhum .out criado
esperar_erro() {
    limpar
    saida=$(./scheduler rate "$1" 2>"$tmp/err"); rc=$?
    if [ "$rc" -ne 0 ] && [ -s "$tmp/err" ] && [ -z "$saida" ] && [ ! -e rate_jems2.out ]; then
        echo "ok     $2  ->  $(cat "$tmp/err")"; ok=$((ok + 1))
    else
        echo "FALHA  $2  (rc=$rc, stdout='$saida')"; falha=$((falha + 1))
    fi
}

esperar_ok tests/vetores/voo.txt "voo.txt valido parseia"

printf 'abc\n'                 > "$tmp/a"; esperar_erro "$tmp/a" "total nao numerico"
printf '0\nATT 20 12 8\n'      > "$tmp/b"; esperar_erro "$tmp/b" "total nao positivo"
printf '100\nATT 20 12\n'      > "$tmp/c"; esperar_erro "$tmp/c" "linha com 3 campos"
printf '100\nATT 20 doze 8\n'  > "$tmp/d"; esperar_erro "$tmp/d" "campo nao numerico"
printf '100\nATT 20 0 8\n'     > "$tmp/e"; esperar_erro "$tmp/e" "campo nao positivo"
printf '100\nATT 20 25 8\n'    > "$tmp/f"; esperar_erro "$tmp/f" "deadline > periodo"
printf '100\nATT 20 12 15\n'   > "$tmp/g"; esperar_erro "$tmp/g" "burst > deadline"
printf '100\n'                 > "$tmp/h"; esperar_erro "$tmp/h" "arquivo sem tarefas"

limpar
echo
echo "resultado: $ok ok, $falha falha"
[ "$falha" -eq 0 ]
