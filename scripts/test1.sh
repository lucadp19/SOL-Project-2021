#!/bin/bash

SOCK_NAME=socket
CONFIG=test1_config.txt
GREEN="\033[0;32m"
NORMC="\033[0m"

echo -e "${GREEN}\t\t TEST 1: testing several client options\n"

touch ${CONFIG}
echo -e "no_worker=1\nsock_path=${SOCK_NAME}\nmax_files=10000\nmax_space=128\npath_dlog=logs" > ${CONFIG}

echo -e "${GREEN}\n\t[CLIENT 0] Prints helper message.${NORMC}\n";
./bin/client -h

echo -e "${GREEN}\n\tOpening server process.${NORMC}\n";
valgrind --leak-check=full ./bin/server ${CONFIG} &
SERVER_PID=$!
sleep 1 # just to make valgrind print stuff

echo -e "${GREEN}\n\t[CLIENT 1] Tests -W and -c.${NORMC}\n";
./bin/client -f ${SOCK_NAME} -p -t 200 -W $PWD/tests/hashtable_test.c,$PWD/tests/list_test.c -D $PWD/tests/test1/deleted -c $PWD/tests/list_test.c
echo -e "${GREEN}\n\t[CLIENT 2] Tests -w.${NORMC}\n";
./bin/client -f ${SOCK_NAME} -p -t 200 -w $PWD/tests/test1/path -D $PWD/tests/test1/deleted
echo -e "${GREEN}\n\t[CLIENT 3] Tests -R and -r.${NORMC}\n";
./bin/client -f ${SOCK_NAME} -p -t 200 -R -d $PWD/tests/test1/readnfiles -r $PWD/tests/hashtable_test.c -d $PWD/tests/test1/readsingle

sleep 2
kill -SIGHUP ${SERVER_PID}
rm -f ${CONFIG}

echo -e "${GREEN}\n\tEnded test!${NORMC}\n"