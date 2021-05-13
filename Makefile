.PHONY: test1 test2

# Compilation options
CC 				= gcc
CFLAGS 			+= -std=c99 -Wall -pedantic -g
THREAD_FLAGS 	= -lpthread
INCLUDE_FLAGS 	= -I./includes

# Directories
SRC_DIR		= ./src
OBJ_DIR		= ./obj
LIB_DIR		= ./libs
INC_DIR		= ./includes
BIN_DIR		= ./bin
TEST_DIR	= ./tests

$(LIB_DIR)/libds.so : $(OBJ_DIR)/list.o $(OBJ_DIR)/node.o
	$(CC) $(CFLAGS) $(INCLUDE_FLAGS) -shared $^ -o $@ 

$(OBJ_DIR)/list.o : $(SRC_DIR)/ds/list.c $(INC_DIR)/list.h $(OBJ_DIR)/node.o
	$(CC) $(CFLAGS) $(INCLUDE_FLAGS) $< -c -fPIC -o $@

$(OBJ_DIR)/node.o : $(SRC_DIR)/ds/node.c $(INC_DIR)/node.h
	$(CC) $(CFLAGS) $(INCLUDE_FLAGS) $< -c -fPIC -o $@

# $(LIB_DIR)/libds.so : $(OBJ_DIR)/%.o
# 	 $(CC) $(CFLAGS) -shared -o $@ $^

# .PHONY: clean
# clean:
# 	@echo "Removing object files and executables..."
# 	@rm -rf $(OBJ_DIR)/*
# 	@rm -rf $(BIN_DIR)/*
# 	@echo "Cleaning complete!"

# test1:
# 	@echo "Test1 has not been written yet :("

# test2:
# 	@echo "Test2 has not been written yet :("