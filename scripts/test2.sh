#!/bin/bash

SOCK_NAME=socket
CONFIG=test2_config.txt
POLICY=$1

GREEN="\033[0;32m"
NORMC="\033[0m"

SERVER=./bin/server 
CLIENT=./bin/client

echo -e "${GREEN}\t\t TEST 2: testing server replacement policy, with policy ${POLICY}\n"

touch ${CONFIG}
echo -e "no_worker=4\nsock_path=${SOCK_NAME}\nmax_files=10\nmax_space=1\npath_dlog=logs\ncache_pol=${POLICY}" > ${CONFIG}

echo -e "${GREEN}\n\tOpening server process.${NORMC}\n";
${SERVER} ${CONFIG} & # opening server
SERVER_PID=$! # getting server PID
sleep 1 # just to make valgrind print stuff

echo -e "${GREEN}\n\t[CLIENT 1] Sends to server a lot of small files.${NORMC}\n";
${CLIENT} -f ${SOCK_NAME} -p -t 200 -w tests/test2/small_files -D tests/test2/deleted_1

# sleep 1

echo -e "${GREEN}\n\t[CLIENT 2] Sends to server some big files.${NORMC}\n";
FILES_TO_READ=tests/test2/small_files/file6.txt
IMGS=tests/test2/img/img0.jpg
for (( i=7; i <= 9; i++ ))
do
    FILES_TO_READ=${FILES_TO_READ},tests/test2/small_files/file${i}.txt
done
for (( i=1; i<=3; i++ ))
do
    IMGS=${IMGS},tests/test2/img/img${i}.jpg
done
${CLIENT} -f ${SOCK_NAME} -p -t 200 -W tests/test2/img/img4.jpg -D tests/test2/deleted_2 -r ${FILES_TO_READ} -d tests/test2/read_files -W tests/test2/img/img0.jpg -D tests/test2/deleted_3 -W ${IMGS} -D tests/test2/deleted_4

kill -SIGHUP ${SERVER_PID}
rm -f ${CONFIG}

echo -e "${GREEN}\n\tEnded test!${NORMC}\n"
