#!/bin/bash

SOCK_NAME=socket
CONFIG=test1_config.txt

GREEN="\033[0;32m"
NORMC="\033[0m"

SERVER=./bin/server 
CLIENT=./bin/client

echo -e "${GREEN}\t\t TEST 1: testing several client options\n"

touch ${CONFIG}
echo -e "no_worker=1\nsock_path=${SOCK_NAME}\nmax_files=10000\nmax_space=128\npath_dlog=logs" > ${CONFIG}

echo -e "${GREEN}\n\t[CLIENT 0] Prints helper message.${NORMC}\n";
${CLIENT} -h

echo -e "${GREEN}\n\tOpening server process.${NORMC}\n";
valgrind --leak-check=full ${SERVER} ${CONFIG} & # opening server
SERVER_PID=$! # getting server PID
sleep 1 # just to make valgrind print stuff

echo -e "${GREEN}\n\t[CLIENT 1] Tests -W and -c.${NORMC}\n";
${CLIENT} -f ${SOCK_NAME} -p -t 200 -W tests/hashtable_test.c,tests/list_test.c -D tests/test1/deleted -c tests/list_test.c
echo -e "${GREEN}\n\t[CLIENT 2] Tests -w.${NORMC}\n";
${CLIENT} -f ${SOCK_NAME} -p -t 200 -w tests/test1/path -D tests/test1/deleted
echo -e "${GREEN}\n\t[CLIENT 3] Tests -R and -r.${NORMC}\n";
${CLIENT} -f ${SOCK_NAME} -p -t 200 -R -d tests/test1/readnfiles -r tests/hashtable_test.c -d tests/test1/readsingle

sleep 1
kill -SIGHUP ${SERVER_PID}
sleep 1 # once again to make valgrind print stuff
rm -f ${CONFIG}

echo -e "${GREEN}\n\tEnded test!${NORMC}\n"