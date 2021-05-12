.PHONY: clean test1 test2

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

clean:
	@echo "Removing object files and executables..."
	@rm -rf ${OBJ_DIR}/*
	@rm -rf ${BIN_DIR}/*
	@echo "Cleaning complete!"

test1:
	@echo "Test1 has not been written yet :("

test2:
	@echo "Test2 has not been written yet :("