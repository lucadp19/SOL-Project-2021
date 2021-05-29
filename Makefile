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
LOG_DIR 	= ./logs

# Dynamic linking
DYN_LINK = -L$(LIB_DIR) -Wl,-rpath,$(LIB_DIR)

# Dependency list for client

# 	---------------- Default rule ----------------	#

all : create_dirs $(BIN_DIR)/client $(BIN_DIR)/server

# 	---------------- Debug macro -----------------  #

debug : CFLAGS += -D DEBUG
debug : all

# 	------------------- Client -------------------	#

CLIENT_SRC := $(wildcard $(SRC_DIR)/client/*.c)
CLIENT_OBJ := $(patsubst $(SRC_DIR)/client/%.c, $(OBJ_DIR)/client/%.o, $(CLIENT_SRC))
CLIENT_INC := $(INC_DIR)/client.h

CLIENT_DEPS = $(LIB_DIR)/libutil.so $(LIB_DIR)/libapi.so
CLIENT_LIBS = $(DYN_LINK) -lutil -lapi

$(BIN_DIR)/client : $(CLIENT_OBJ)
	$(CC) $(CFLAGS) $(CLIENT_LIBS) $^ -o $@

$(OBJ_DIR)/client/%.o : $(SRC_DIR)/client/%.c $(CLIENT_INC) $(CLIENT_DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

# 	------------------- Server ------------------- 	#

SERVER_SRC := $(wildcard $(SRC_DIR)/server/*.c)
SERVER_OBJ := $(patsubst $(SRC_DIR)/server/%.c, $(OBJ_DIR)/server/%.o, $(SERVER_SRC))
SERVER_INC := $(INC_DIR)/server.h

SERVER_DEPS := $(LIB_DIR)/libutil.so
SERVER_LIBS := $(DYN_LINK) -lutil

$(BIN_DIR)/server : $(SERVER_OBJ)
	$(CC) $(CFLAGS) $(SERVER_LIBS) $^ -o $@

$(OBJ_DIR)/server/%.o : $(SRC_DIR)/server/%.c $(SERVER_INC) $(SERVER_DEPS)
	$(CC) $(CFLAGS) $< -c -o $@

# 	----------------- Server API -----------------  #

API_SRC := $(wildcard $(SRC_DIR)/api/*.c)
API_OBJ := $(patsubst $(SRC_DIR)/api/%.c, $(OBJ_DIR)/api/%.o, $(API_SRC))
API_INC := $(INC_DIR)/api.h $(wildcard $(INC_DIR)/api/*.h) 

$(LIB_DIR)/libapi.so : $(API_OBJ) $(DEP_LIST)
	$(CC) $(CFLAGS) -shared $(API_OBJ) -o $@

$(OBJ_DIR)/api/%.o : $(SRC_DIR)/api/%.c $(API_INC) $(DEP_LIST)
	$(CC) -fPIC $(CFLAGS) $< -c -o $@

#	-------------- Utilities Library -------------  #

UTIL_SRC := $(wildcard $(SRC_DIR)/util/*.c) $(wildcard $(SRC_DIR)/util/*/*.c)
UTIL_OBJ := $(patsubst $(SRC_DIR)/util/%.c, $(OBJ_DIR)/util/%.o, $(UTIL_SRC))
UTIL_INC := $(patsubst $(SRC_DIR)/util/%.c, $(INC_DIR)/util/%.h, $(UTIL_SRC))

$(LIB_DIR)/libutil.so : $(UTIL_OBJ)
	$(CC) $(CFLAGS) -shared $^ -o $@

$(OBJ_DIR)/util/%.o : $(SRC_DIR)/util/%.c $(INC_DIR)/util/%.h  $(OBJ_DIR)/util/util.o
	$(CC) $(CFLAGS) $< -c -fPIC -o $@

$(OBJ_DIR)/util/hash/%.o :  $(SRC_DIR)/util/hash/%.c $(INC_DIR)/util/hash/%.h $(OBJ_DIR)/util/util.o
	$(CC) $(CFLAGS) $< -c -fPIC -o $@

$(OBJ_DIR)/util/util.o : $(SRC_DIR)/util/util.c $(INC_DIR)/util/util.h
	$(CC) $(CFLAGS) $< -c -fPIC -o $@

#	--------------- Directory utils --------------	#

.PHONY: create_dirs
create_dirs:
	@bash scripts/create_dirs.sh
	
# 	------------------ Cleaning ------------------	#

.PHONY: clean
clean:
	@echo "Removing object files and executables..."
	@rm -f vgcore.*
	@rm -rf $(OBJ_DIR)/*
	@rm -rf $(BIN_DIR)/*
	@rm -rf $(LOG_DIR)/*
	@echo "Cleaning complete!"

# ---------------- Personal Tests ---------------- 	#
.PHONY: list_test hashtable_test

PTEST_DEPS := $(LIB_DIR)/libutil.so
PTEST_LIBS := $(DYN_LINK) -lutil

hashtable_test : $(BIN_DIR)/hashtable_test

$(BIN_DIR)/hashtable_test : $(TEST_DIR)/hashtable_test.c $(LIB_DIR)/libutil.so
	$(CC) $(CFLAGS) $(PTEST_LIBS) $< -o $@

list_test: $(BIN_DIR)/list_test

$(BIN_DIR)/list_test: $(TEST_DIR)/list_test.c $(LIB_DIR)/libutil.so
	$(CC) $(CFLAGS) $(PTEST_LIBS) $< -o $@

# ---------------- Official Tests ----------------	#
.PHONY : test1 test2

test1:
	@echo "Test1 has not been written yet :("

test2:
	@echo "Test2 has not been written yet :("