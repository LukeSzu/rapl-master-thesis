#!/bin/bash
set -euo pipefail

MAX_POW_W=$(($(cat /sys/class/powercap/intel-rapl:0/constraint_0_max_power_uw) / 1000000))

DOMS=()
max_energy=()
for d in /sys/class/powercap/intel-rapl:*; do
    [[ -d "$d" ]] || continue
    [[ "$d" =~ ^.*/intel-rapl:[0-9]+$ ]] || continue
    [[ -f "$d/energy_uj" ]] || continue
    DOMS+=("$d")
    max_energy+=( "$(cat "$d/max_energy_range_uj")" )
done

if [[ ${#DOMS[@]} -eq 0 ]]; then
    echo "No RAPL domains"
    exit 1
fi

energy_before=()
energy_after=()
energy_used_proc=()
energy_used=0
energy_used_yoko=0

set_rapl(){
    local LIMIT_WATTS=$1
    local TIME_SEC=$2

    POWER_LIMIT=$((LIMIT_WATTS * 1000000))    
    TIME_WINDOW=$((TIME_SEC* 1000000))  

    for RAPL_PATH in "${DOMS[@]}"; do
        # set power limit
        if ! echo "$POWER_LIMIT" > "$RAPL_PATH/constraint_0_power_limit_uw"; then
            echo "Error: failed to set power limit ($LIMIT_WATTS W)"
            return 1
        fi
        # set time window
        if ! echo "$TIME_WINDOW" > "$RAPL_PATH/constraint_0_time_window_us"; then
            echo "Error: failed to set time window ($TIME_SEC s)"
            return 1
        fi
    done
    return 0
}
measure_energy(){
    energy_used=0

    for i in "${!DOMS[@]}"; do
        local before="${energy_before[$i]}"
        local after="${energy_after[$i]}"
        local max="${max_energy[$i]}"
        local used

        if (( after >= before )); then
            used=$(( after - before ))
        else
            used=$(( (max - before) + after ))
        fi
        energy_used_proc[$i]="$used"
        (( energy_used += used ))
    done

    #energy_used_yoko=$(yokotool read Wh --count 1 | tail -n1 | awk '{if($1<0){$1=-$1} print $1*3600}')
}
measure_energy_before(){
    for i in "${!DOMS[@]}"; do
        energy_before[$i]="$(cat "${DOMS[$i]}/energy_uj")"
    done
}
measure_energy_after(){
    for i in "${!DOMS[@]}"; do
        energy_after[$i]="$(cat "${DOMS[$i]}/energy_uj")"
    done
}

NPROC=$(nproc)
REPEATS=10

#Lists for every program
POWER_LIST=()
n=10
for ((i=0; i<n; i++)); do
    val=$(( MAX_POW_W * (n - i) / n ))
    POWER_LIST+=($val)
done

THREADS_LIST=($((NPROC/4)) $((NPROC/2)) $NPROC $((2*NPROC)))
BUFFER_LIST=($((2*NPROC*500*2)))
MODEL_LIST=(0 1 2)

#Lists for Merge sort
SORT_SIZE_LIST=(1000000 10000000 100000000)

#Lists for Matrix determinant 
MATRIX_SIZE_LIST=(2000 3000 4000)

#Lists for Mandelbrot's set arguments
MANDELBROT_SIZE_LIST=(1000 2000 3000)

#Total benchmarks
total_all=$(( ${#POWER_LIST[@]} * ${#THREADS_LIST[@]} * ${#BUFFER_LIST[@]} * ${#MODEL_LIST[@]}))
total_sort=$(( total_all * REPEATS * ${#SORT_SIZE_LIST[@]} ))
total_matrix=$(( total_all * REPEATS * ${#MATRIX_SIZE_LIST[@]} ))
total_mandelbrot=$(( total_all * REPEATS * ${#MANDELBROT_SIZE_LIST[@]} ))


#Matrix determinant
OUTPUT_Matrix="mx_results.csv"
if [ ! -f "$OUTPUT_Matrix" ]; then
    echo "model, npl, buffer_size, problem_size, power_limit, energy_used_RAPL, energy_used_yoko, working_time" >> "$OUTPUT_Matrix"
fi

counter=1
for current_power in "${POWER_LIST[@]}"; do
    set_rapl "$current_power" 10 || exit 1
    sleep 1
    for current_npl in "${THREADS_LIST[@]}"; do
        for current_buffer in "${BUFFER_LIST[@]}"; do
            for current_model in "${MODEL_LIST[@]}"; do
                for current_size in "${MATRIX_SIZE_LIST[@]}"; do
                    for ((rep=1; rep<=REPEATS; rep++)); do
                        echo "Matrix determinant | [$((counter++))/$total_matrix] | power: $current_power, threads: $current_npl, buffer: $current_buffer, model: $current_model, size: $current_size"

                        #yokotool integration reset
                        start_ns=$(date +%s%N)
                        measure_energy_before

                        #yokotool integration start
                        ./mgr_release Mx -t $current_npl -bs $current_buffer -model $current_model -size $current_size
                        #yokotool integration stop

                        measure_energy_after
                        end_ns=$(date +%s%N)
                        measure_energy

                        dt_ns=$((end_ns - start_ns))
                        working_time=$(awk -v ns="$dt_ns" 'BEGIN{printf("%.9f", ns/1e9)}')

                        echo "$current_model, $current_npl, $current_buffer, $current_size, $current_power, $energy_used, $energy_used_yoko, $working_time" >> "$OUTPUT_Matrix"
                    done
                done
            done
        done
    done
done

counter=1
#Mandelbrot
OUTPUT_Mandelbrot="mb_results.csv"
if [ ! -f "$OUTPUT_Mandelbrot" ]; then
    echo "model, npl, buffer_size, problem_size, power_limit, energy_used_RAPL, energy_used_yoko, working_time" >> "$OUTPUT_Mandelbrot"
fi

for current_power in "${POWER_LIST[@]}"; do
    set_rapl "$current_power" 10 || exit 1
    sleep 1
    for current_npl in "${THREADS_LIST[@]}"; do
        for current_buffer in "${BUFFER_LIST[@]}"; do
            for current_model in "${MODEL_LIST[@]}"; do
                for current_size in "${MANDELBROT_SIZE_LIST[@]}"; do
                    for ((rep=1; rep<=REPEATS; rep++)); do
                        echo "Mandelbrot | [$((counter++))/$total_mandelbrot] | power: $current_power, threads: $current_npl, buffer: $current_buffer, model: $current_model, size: $current_size"

                        #yokotool integration reset
                        start_ns=$(date +%s%N)
                        measure_energy_before

                        #yokotool integration start
                        ./mgr_release Mb -t $current_npl -bs $current_buffer -model $current_model -w $current_size -h $current_size -it 15000 -b 100
                        #yokotool integration stop

                        measure_energy_after
                        end_ns=$(date +%s%N)
                        measure_energy
                        dt_ns=$((end_ns - start_ns))
                        working_time=$(awk -v ns="$dt_ns" 'BEGIN{printf("%.9f", ns/1e9)}')

                        echo "$current_model, $current_npl, $current_buffer, $current_size, $current_power, $energy_used, $energy_used_yoko, $working_time" >> "$OUTPUT_Mandelbrot"
                    done
                done
            done
        done
    done
done

#Merge sort
OUTPUT_MergeSort="ms_results.csv"
if [ ! -f "$OUTPUT_MergeSort" ]; then
    echo "model, npl, buffer_size, problem_size, power_limit, energy_used_RAPL, energy_used_yoko, working_time" >> "$OUTPUT_MergeSort"
fi

counter=1
for current_power in "${POWER_LIST[@]}"; do
    set_rapl "$current_power" 10 || exit 1
    sleep 1
    for current_npl in "${THREADS_LIST[@]}"; do
        for current_buffer in "${BUFFER_LIST[@]}"; do
            for current_model in "${MODEL_LIST[@]}"; do
                for current_size in "${SORT_SIZE_LIST[@]}"; do
                    for ((rep=1; rep<=REPEATS; rep++)); do
                        echo "Merge sort | [$((counter++))/$total_sort] | power: $current_power, threads: $current_npl, buffer: $current_buffer, model: $current_model, size: $current_size"

                        #yokotool integration reset
                        start_ns=$(date +%s%N)
                        measure_energy_before

                        #yokotool integration start
                        ./mgr_release St -t $current_npl -bs $current_buffer -model $current_model -size $current_size
                        #yokotool integration stop

                        measure_energy_after
                        end_ns=$(date +%s%N)
                        measure_energy
                        dt_ns=$((end_ns - start_ns))
                        working_time=$(awk -v ns="$dt_ns" 'BEGIN{printf("%.9f", ns/1e9)}')

                        echo "$current_model, $current_npl, $current_buffer, $current_size, $current_power, $energy_used, $energy_used_yoko, $working_time" >> "$OUTPUT_MergeSort"
                    done
                done
            done
        done
    done
done











