# Compilation options
CC 		= gcc
CFLAGS 	+= -std=c99 -Wall -pedantic -g -I./includes -lpthread -lm -D_POSIX_C_SOURCE=200112L

# Directories
SRC_DIR		= ./src
OBJ_DIR		= ./obj
LIB_DIR		= ./libs
INC_DIR		= ./includes
BIN_DIR		= ./bin
TEST_DIR	= ./tests

# Dynamic linking
DYN_LINK = -L$(LIB_DIR) -Wl,-rpath,$(LIB_DIR)

# Dependency list for applications
DEP_LIST = $(LIB_DIR)/libds.so $(OBJ_DIR)/util.o

# 	---------------- Default rule ----------------	#
all : $(BIN_DIR)/client $(BIN_DIR)/server

# 	---------------- Debug macro -----------------  #
debug : CFLAGS += -D DEBUG
debug : all

# 	------------------- Client -------------------	#

$(BIN_DIR)/client : $(SRC_DIR)/client.c  $(INC_DIR)/client.h $(DEP_LIST)
	$(CC) $(CFLAGS) $(DYN_LINK) -lds $< $(DEP_LIST) -o $@

# 	------------------- Server ------------------- 	#
SERVER_SRC := $(wildcard $(SRC_DIR)/server/*.c)
SERVER_OBJ := $(patsubst $(SRC_DIR)/server/*.c, $(OBJ_DIR)/server/*.o, $(SERVER_SRC))

$(BIN_DIR)/server : $(SERVER_OBJ) $(DEP_LIST)
	$(CC) $(CFLAGS) $(DYN_LINK) -lds $^ -o $@

$(OBJ_DIR)/server/%.o : $(SRC_DIR)/server/%.c $(INC_DIR)/server.h $(DEP_LIST)
	$(CC) $(CFLAGS) $< -c -o $@

#	---------- Data Structures Library	----------  #

$(LIB_DIR)/libds.so : $(OBJ_DIR)/list.o $(OBJ_DIR)/node.o $(OBJ_DIR)/hashtable.o $(OBJ_DIR)/hash.o
	$(CC) $(CFLAGS) -shared $^ -o $@ 

$(OBJ_DIR)/hash.o : $(SRC_DIR)/ds/hash.c $(INC_DIR)/hash.h $(OBJ_DIR)/hashtable.o $(OBJ_DIR)/hashmap.o
	$(CC) $(CFLAGS) $< -c -fPIC -o $@

$(OBJ_DIR)/hashmap.o : $(SRC_DIR)/ds/hashmap.c $(INC_DIR)/hash.h $(OBJ_DIR)/list.o
	$(CC) $(CFLAGS) $< -c -fPIC -o $@

$(OBJ_DIR)/hashtable.o : $(SRC_DIR)/ds/hashtable.c $(INC_DIR)/hash.h $(OBJ_DIR)/list.o
	$(CC) $(CFLAGS) $< -c -fPIC -o $@

$(OBJ_DIR)/list.o : $(SRC_DIR)/ds/list.c $(INC_DIR)/list.h $(OBJ_DIR)/node.o
	$(CC) $(CFLAGS) $< -c -fPIC -o $@

$(OBJ_DIR)/node.o : $(SRC_DIR)/ds/node.c $(INC_DIR)/node.h
	$(CC) $(CFLAGS) $< -c -fPIC -o $@

#  	----------------- Utilities ------------------  #

$(OBJ_DIR)/util.o : $(SRC_DIR)/util/util.c $(INC_DIR)/util.h
	$(CC) $(CFLAGS) -c $< -o $@
	
# Cleaning
.PHONY: clean
clean:
	@echo "Removing object files and executables..."
	@rm -f vgcore.*
	@rm -rf $(OBJ_DIR)/*
	@rm -rf $(BIN_DIR)/*
	@echo "Cleaning complete!"

# Tests
.PHONY: list_test test1 test2

hashtable_test : $(BIN_DIR)/hashtable_test

$(BIN_DIR)/hashtable_test : $(TEST_DIR)/hashtable_test.c $(DEP_LIST)
	$(CC) $(CFLAGS) $(DYN_LINK) $(DEBUG) -lds $< $(OBJ_DIR)/util.o -o $@

list_test: $(BIN_DIR)/list_test

$(BIN_DIR)/list_test: $(TEST_DIR)/list_test.c $(DEP_LIST)
	$(CC) $(CFLAGS) $(DYN_LINK) $(DEBUG) -lds $< $(OBJ_DIR)/util.o -o $@

test1:
	@echo "Test1 has not been written yet :("

test2:
	@echo "Test2 has not been written yet :("