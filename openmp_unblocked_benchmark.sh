set -euo pipefail
export LC_ALL=C

cd "$(dirname "${BASH_SOURCE[0]}")"

TRIALS="${1:-20}"
WARMUPS="${2:-1}"
THREAD_COUNTS=(1 2 4 8 16)
SOURCE="openmp_unblocked.c"
EXECUTABLE="./openmp_unblocked"
CSV="openmp_unblocked_results.csv"
NODE="$(hostname -s)"

if ! [[ "$TRIALS" =~ ^[1-9][0-9]*$ && "$WARMUPS" =~ ^[0-9]+$ ]]; then
    echo "Usage: bash $0 [positive_number_of_trials] [nonnegative_warmups]" >&2
    exit 1
fi

if [[ "$NODE" != "zeus-1" ]]; then
    echo "WARNING: Running on $NODE, not zeus-1." >&2
fi

if [[ ! -f "$SOURCE" ]]; then
    echo "ERROR: Cannot find $SOURCE in $(pwd)." >&2
    exit 1
fi

echo "Compiling $SOURCE with -O0 and OpenMP (without -g)..."
make -B openmp_unblocked

# These correctness checks are separate from the measured benchmark trials.
for threads in "${THREAD_COUNTS[@]}"; do
    echo "Checking correctness with $threads thread(s)..."
    verify_output="$("$EXECUTABLE" "$threads" --verify)"
    if ! grep -Fq 'Reference validation: PASS (0 mismatches)' <<< "$verify_output"; then
        echo "$verify_output" >&2
        echo "ERROR: Verification failed for $threads threads; no CSV was written." >&2
        exit 1
    fi
done
echo "Correctness verification passed for every thread count."

echo "Performing $WARMUPS warm-up run(s) per thread count..."
for threads in "${THREAD_COUNTS[@]}"; do
    for ((w=1; w<=WARMUPS; w++)); do
        "$EXECUTABLE" "$threads" >/dev/null
    done
done

# A fresh file is written for each benchmark session.
printf 'trial,threads,execution_time_seconds,checksum,node\n' > "$CSV"
expected_checksum=""

echo "Starting $TRIALS measured trial(s) per thread count on $NODE..."
for ((trial=1; trial<=TRIALS; trial++)); do
    # Randomize thread-count order each round to reduce order-related bias.
    order=("${THREAD_COUNTS[@]}")
    for ((i=${#order[@]}-1; i>0; i--)); do
        j=$((RANDOM % (i+1)))
        temp="${order[i]}"
        order[i]="${order[j]}"
        order[j]="$temp"
    done

    for threads in "${order[@]}"; do
        output="$("$EXECUTABLE" "$threads")"
        actual_threads="$(awk '/^Threads:/ {print $2; exit}' <<< "$output")"
        seconds="$(awk '/^Execution time:/ {print $3; exit}' <<< "$output")"
        checksum="$(awk '/^Checksum:/ {print $2; exit}' <<< "$output")"

        if [[ "$actual_threads" != "$threads" ||
              ! "$seconds" =~ ^[0-9]+([.][0-9]+)?$ ||
              -z "$checksum" ]]; then
            echo "$output" >&2
            echo "ERROR: Could not parse benchmark output." >&2
            exit 1
        fi

        if [[ -z "$expected_checksum" ]]; then
            expected_checksum="$checksum"
        elif [[ "$checksum" != "$expected_checksum" ]]; then
            echo "ERROR: Checksum changed at trial $trial, $threads threads." >&2
            exit 1
        fi

        printf '%d,%d,%s,%s,%s\n' \
            "$trial" "$threads" "$seconds" "$checksum" "$NODE" >> "$CSV"
        printf 'Trial %2d/%d | %2d threads | %s seconds\n' \
            "$trial" "$TRIALS" "$threads" "$seconds"
    done
done

echo
echo "Complete: $((TRIALS * ${#THREAD_COUNTS[@]})) measurements saved in $CSV"
