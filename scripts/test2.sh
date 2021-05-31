#!/bin/bash

SOCK_NAME=socket
CONFIG=test2_config.txt

GREEN="\033[0;32m"
NORMC="\033[0m"

SERVER=./bin/server 
CLIENT=./bin/client

echo -e "${GREEN}\t\t TEST 2: testing server replacement policy\n"

touch ${CONFIG}
echo -e "no_worker=4\nsock_path=${SOCK_NAME}\nmax_files=10\nmax_space=1\npath_dlog=logs" > ${CONFIG}

echo -e "${GREEN}\n\tOpening server process.${NORMC}\n";
${SERVER} ${CONFIG} & # opening server
SERVER_PID=$! # getting server PID
sleep 1 # just to make valgrind print stuff

echo -e "${GREEN}\n\t[CLIENT 1] Sends to server a lot of small files.${NORMC}\n";
${CLIENT} -f ${SOCK_NAME} -p -t 200 -w tests/test2/small_files -D tests/test2/deleted_1

sleep 1

echo -e "${GREEN}\n\t[CLIENT 2] Sends to server some big files.${NORMC}\n";
${CLIENT} -f ${SOCK_NAME} -p -t 200 -w tests/test2/img -D tests/test2/deleted_2

kill -SIGHUP ${SERVER_PID}
rm -f ${CONFIG}

echo -e "${GREEN}\n\tEnded test!${NORMC}\n"
