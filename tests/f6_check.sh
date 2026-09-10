#!/bin/sh
set -u

cd "$(dirname "$0")/.." || exit 1
make || { echo "make FALHOU"; exit 1; }

vetor=tests/vetores/rm_falha_edf_ok.txt
esp_rate=tests/vetores/rm_falha_edf_ok_esperado_rate.out
esp_edf=tests/vetores/rm_falha_edf_ok_esperado_edf.out
ok=0
falha=0

rm -f rate_jems2.out edf_jems2.out

# soma os numeros do bloco LOST DEADLINES de um .out
perdidos() {
    soma=0
    for n in $(sed -n '/^LOST DEADLINES$/,/^$/p' "$1" | sed -n 's/^\[.*\] \([0-9]*\)$/\1/p'); do
        soma=$((soma + n))
    done
    echo "$soma"
}

./scheduler rate "$vetor" || { echo "FALHA  scheduler rate saiu != 0"; falha=$((falha + 1)); }
./scheduler edf  "$vetor" || { echo "FALHA  scheduler edf saiu != 0";  falha=$((falha + 1)); }

lr=$(perdidos rate_jems2.out)
le=$(perdidos edf_jems2.out)

if [ "$lr" -gt 0 ]; then
    echo "ok     RATE perde deadline (LOST total = $lr)"; ok=$((ok + 1))
else
    echo "FALHA  RATE devia perder deadline (LOST total = $lr)"; falha=$((falha + 1))
fi

if [ "$le" -eq 0 ]; then
    echo "ok     EDF nao perde nenhum deadline (LOST total = $le)"; ok=$((ok + 1))
else
    echo "FALHA  EDF nao devia perder deadline (LOST total = $le)"; falha=$((falha + 1))
fi

# regressao byte a byte contra os vetores guardados
if cmp -s rate_jems2.out "$esp_rate"; then
    echo "ok     rate_jems2.out bate com $esp_rate"; ok=$((ok + 1))
else
    echo "FALHA  rate_jems2.out difere de $esp_rate"; falha=$((falha + 1))
fi

if cmp -s edf_jems2.out "$esp_edf"; then
    echo "ok     edf_jems2.out bate com $esp_edf"; ok=$((ok + 1))
else
    echo "FALHA  edf_jems2.out difere de $esp_edf"; falha=$((falha + 1))
fi

rm -f rate_jems2.out edf_jems2.out
echo
echo "resultado: $ok ok, $falha falha"
[ "$falha" -eq 0 ]
