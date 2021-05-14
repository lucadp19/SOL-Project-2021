# Compilation options
CC 				= gcc
CFLAGS 			+= -std=c99 -Wall -pedantic -g -I./includes
THREAD_FLAGS 	= -lpthread

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

#  	----------------- Utilities ------------------  #

$(OBJ_DIR)/util.o : $(SRC_DIR)/util/util.c $(INC_DIR)/util.h
	$(CC) $(CFLAGS) -c $< -o $@

#	---------- Data Structures Library	----------  #

$(LIB_DIR)/libds.so : $(OBJ_DIR)/list.o $(OBJ_DIR)/node.o
	$(CC) $(CFLAGS) -shared $^ -o $@ 

$(OBJ_DIR)/list.o : $(SRC_DIR)/ds/list.c $(INC_DIR)/list.h $(OBJ_DIR)/node.o
	$(CC) $(CFLAGS) $< -c -fPIC -o $@

$(OBJ_DIR)/node.o : $(SRC_DIR)/ds/node.c $(INC_DIR)/node.h
	$(CC) $(CFLAGS) $< -c -fPIC -o $@

# Cleaning
.PHONY: clean
clean:
	@echo "Removing object files and executables..."
	@rm -rf $(OBJ_DIR)/*
	@rm -rf $(BIN_DIR)/*
	@echo "Cleaning complete!"

# Tests
.PHONY: list_test test1 test2
list_test: $(BIN_DIR)/list_test

$(BIN_DIR)/list_test: $(TEST_DIR)/list_test.c $(DEP_LIST)
	$(CC) $(CFLAGS) $(DYN_LINK) $(DEBUG) -lds $< $(OBJ_DIR)/util.o -o $@

test1:
	@echo "Test1 has not been written yet :("

test2:
	@echo "Test2 has not been written yet :("