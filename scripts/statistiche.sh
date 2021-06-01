#!/bin/bash

# to exit on pipe when checking if logs exist
set -o pipefail

if [ $# -eq 0 ]; # no additional argument => using last log file in ./logs/
  then
    if [ -d logs ];
      then
        LOG_FILE=logs/$(ls logs | grep -E 'log-*' | tail -n1) || { echo "No log file could be found inside logs/ directory. Aborting script."; exit 1; }  
    else # log
        echo "No log file could be found. Aborting script."
        exit 1
    fi
else
    LOG_FILE=$1
fi

# unsetting pipefail
set +o pipefail

GREEN="\033[0;32m"
NORMC="\033[0m"

# ----- READ DATA ----- #
echo -e "\t${GREEN}Data read by clients.${NORMC}\n"

n_reads=$(grep "\[READ_FILE_SUCCESS\]" -c $LOG_FILE)
n_readNFiles=$(grep "\[READ_N_FILES_SUCCESS\]" -c $LOG_FILE)
tot_bytes_read=$(grep -Eo "\[WRITE_TO_CLIENT\]\[READ_FILE\]\[WB\] [0-9]+" ${LOG_FILE} | grep -Eo "[0-9]+" | { sum=0; while read num; do ((sum+=num)); done; echo $sum; } )
tot_bytes_readNFiles=$(grep -Eo "\[WRITE_TO_CLIENT\]\[READ_N_FILES\]\[WB\] [0-9]+" ${LOG_FILE} | grep -Eo "[0-9]+" | { sum=0; while read num; do ((sum+=num)); done; echo $sum; } )

echo "No. of successful readFile: ${n_reads}"
echo "No. of bytes read by readFile: ${tot_bytes_read}"
if [ ${n_reads} -gt 0 ]; then
    mean_bytes_read=$(echo "scale=4; ${tot_bytes_read} / ${n_reads}" | bc -l)
    echo "Mean number of bytes read by readFile: ${mean_bytes_read}"
fi

echo ""

echo "No. of successful readNFiles: ${n_readNFiles}"
echo "No. of bytes read by readNFiles: ${tot_bytes_readNFiles}"
if [[ ${n_readNFiles} -gt 0 ]]; then
    mean_bytes_readNFiles=$(echo "scale=4; ${tot_bytes_readNFiles} / ${n_readNFiles}" | bc -l)
    echo "Mean number of bytes read by readNFiles: ${mean_bytes_readNFiles}"
fi

echo ""

echo "Total number of bytes read by clients: $(( tot_bytes_read + tot_bytes_readNFiles))"
if (( n_reads + n_readNFiles > 0 )); then
    mean_bytes_gen_read=$(echo "scale=4; ($tot_bytes_read + $tot_bytes_readNFiles) / ($n_reads + $n_readNFiles)" | bc -l )
    echo "Mean number of bytes read by clients: ${mean_bytes_gen_read}"
fi


# ------ WRITES ------ #
echo -e "\n\t${GREEN}Data written by clients.${NORMC}\n"

n_writes=$(grep "\[WRITE_FILE_SUCCESS\]\[WB\]" -c $LOG_FILE)
n_appends=$(grep "\[APPEND_TO_FILE_SUCCESS\]\[WB\]" -c $LOG_FILE)
tot_bytes_writes=$(grep -Eo "\[WRITE_FILE_SUCCESS\]\[WB\] [0-9]+" ${LOG_FILE} | grep -Eo "[0-9]+" | { sum=0; while read num; do ((sum+=num)); done; echo $sum; } )
tot_bytes_appends=$(grep -Eo "\[APPEND_TO_FILE_SUCCESS\]\[READ_N_FILES\]\[WB\] [0-9]+" ${LOG_FILE} | grep -Eo "[0-9]+" | { sum=0; while read num; do ((sum+=num)); done; echo $sum; } )

echo "No. of successful writeFile: ${n_writes}"
echo "No. of bytes written by writeFile: ${tot_bytes_writes}"
if [ ${n_writes} -gt 0 ]; then
    mean_bytes_written=$(echo "scale=4; ${tot_bytes_writes} / ${n_writes}" | bc -l)
    echo "Mean number of bytes written by writeFile: ${mean_bytes_written}"
fi

echo ""

echo "No. of successful appendToFile: ${n_writes}"
echo "No. of bytes written by appendToFile: ${tot_bytes_appends}"
if [[ ${n_appends} -gt 0 ]]; then
    mean_bytes_appends=$(echo "scale=4; ${tot_bytes_appends} / ${n_appends}" | bc -l)
    echo "Mean number of bytes written by appendToFile: ${mean_bytes_appends}"
fi

echo ""

echo "Total number of bytes written by clients: $(( tot_bytes_writes + tot_bytes_appends))"
if (( n_appends + n_writes > 0 )); then
    mean_bytes_gen_write=$(echo "scale=4; ($tot_bytes_appends + $tot_bytes_writes) / ($n_appends + $n_writes)" | bc -l )
    echo "Mean number of bytes written by clients: ${mean_bytes_gen_write}"
fi

# ----- REPLACEMENT ----- #
echo -e "\n\t${GREEN}Expelled files.${NORMC}\n"

n_repl=$(grep "\[REPLACEMENT\]\[BF]" -c $LOG_FILE)
tot_bytes_repl=$(grep -Eo "\[REPLACEMENT\]\[BF\] [0-9]+" ${LOG_FILE} | grep -Eo "[0-9]+" | { sum=0; while read num; do ((sum+=num)); done; echo $sum; } )
tot_bytes_repl_sent=$(grep -Eo "\[WRITE_TO_CLIENT\]\[REPLACEMENT\]\[WB\] [0-9]+" ${LOG_FILE} | grep -Eo "[0-9]+" | { sum=0; while read num; do ((sum+=num)); done; echo $sum; } )
n_throw_away=$(grep "\[THROWN_AWAY\]" -c $LOG_FILE)
n_repl_sent=$((n_repl - n_throw_away))

echo "No. of replaced files: $n_repl"
echo "Total bytes of replaced files: $tot_bytes_repl"
echo "No. of thrown away files: $n_throw_away"
echo "No. of replaced files sent to clients: $n_repl_sent"
echo "Total bytes sent to clients as a result of replacements: $tot_bytes_repl_sent"
if [[ ${n_repl_sent} -gt 0 ]]; then
    mean_bytes_repl=$(echo "scale=4; ${tot_bytes_repl_sent} / ${n_repl_sent}" | bc -l)
    echo "Mean number of bytes per file sent to clients as a result of replacements: ${mean_bytes_repl}"
fi

# ----- STATS ----- #
echo -e "\n\t${GREEN}More general stats.${NORMC}\n"

n_locks=$(grep -Eo "\[LOCK_FILE_SUCCESS\]" -c $LOG_FILE)
n_openlock=$(grep -Eo "\[OPEN_FILE_SUCCESS\]\[LOCK\]" -c $LOG_FILE)
n_unlock=$(grep -Eo "\[UNLOCK_FILE_SUCCESS\]" -c $LOG_FILE)
n_close=$(grep -Eo "\[CLOSE_FILE_SUCCESS\]" -c $LOG_FILE)
max_size=$(grep -Eo "\[STATS\]\[CURRENT_SPACE\] [0-9]+" ${LOG_FILE} | grep -Eo "[0-9]+" | { max=0; while read num; do if (( max<num )); then ((max=num)); fi; done; echo $max; } )
max_files=$(grep -Eo "\[CURRENT_FILES\] [0-9]+" ${LOG_FILE} | grep -Eo "[0-9]+" | { max=0; while read num; do if (( max<num )); then ((max=num)); fi; done; echo $max; } )
max_conn=$(grep -Eo "\[STATS\]\[CURRENT_CONNECTIONS\] [0-9]+" ${LOG_FILE} | grep -Eo "[0-9]+" | { max=0; while read num; do if (( max<num )); then ((max=num)); fi; done; echo $max; } )

echo "No. of locks: $n_locks"
echo "No. of open-locks: $n_openlock"
echo "No. of unlocks: $n_unlock"
echo "No. of close: $n_close"
echo "Max size reached by server: $max_size"
echo "Max number of files stored: $max_files"
echo "Max number of connected clients: $max_conn"

# getting no. of worker threads
echo -e "\n\t${GREEN}Number of requests fulfilled by each thread.${NORMC}\n"
n_threads=0
while grep -Eo "\[THREAD $n_threads\]" $LOG_FILE > /dev/null; do
    n_threads=$((n_threads+1));
done
for ((i=0; i<n_threads; i++)); do
    n_thread_req=$(grep -Eo "\[THREAD $i\] \[NEW_REQ\]" -c ${LOG_FILE})
    echo "Thread $i fulfilled $n_thread_req requests from clients."
done

echo -e "\n\t${GREEN}End of stats!${NORMC}"