#!/usr/bin/env bash

ERR_FREE=0

if [[ ! -f ./as ]]; then
    make as
fi

declare -A tests

tests["./examples/fibonacci.asm"]=26
tests["./examples/mul12345.asm"]=30

chk_file () {
    ASM_FILE="$1"
    EXP_LINES=$2
    if [[ -f "${ASM_FILE}" ]]; then
        ASM="$(./as "${ASM_FILE}")"
        ACT_LINES="$(wc -l <<< "${ASM}")"
        if [[ ${ACT_LINES} -eq ${EXP_LINES} ]]; then
            echo "[${ASM_FILE}] Correct number of lines!"
        else
            echo "[${ASM_FILE}] Incorrect number of lines! (${ACT_LINES} lines found)"
            ERR_FREE=1
        fi
    fi
}

for item in "${!tests[@]}"; do
    chk_file "${item}" "${tests[${item}]}"
done

exit "${ERR_FREE}"
