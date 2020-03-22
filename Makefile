# BUILD YOUR OWN LISP MAKEFILE
# 

# DIRS
SRC_DIR=./src
OBJ_DIR=./obj
BIN_DIR=./bin
TEST_DIR=./test 
TEST_BIN_DIR=bin/test	
##TEST_BIN_DIR=$(BIN_DIR)/test

# TOOLS 
CC=gcc
OPT=-O0
CFLAGS += -Wall -g2 -std=c99 -D_REENTRANT $(OPT)
CFLAGS += 
LDFLAGS=
LIBS=-lm -ledit
TEST_LIBS=-lcheck

INCS=-I$(SRC_DIR)
# Sources
SOURCES=$(wildcard $(SRC_DIR)/*.c)
# Unit test sources 
TEST_SOURCES=$(wildcard test/*.c)	# issue here with directory name..

# Source objects  
OBJECTS := $(SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
$(OBJECTS): $(OBJ_DIR)/%.o : $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $(INCS) -c $< -o $@

# Test objects
TEST_OBJECTS  := $(TEST_SOURCES:test/%.c=$(OBJ_DIR)/%.o)
$(TEST_OBJECTS): $(OBJ_DIR)/%.o : test/%.c 
	$(CC) $(CFLAGS) $(INCS) -c $< -o $@ 


.PHONY: clean

repl: $(OBJECTS)
	$(CC) $(LDFLAGS) $(INCS) $(OBJECTS) -o $(BIN_DIR)/repl $(LIBS)

all: repl test

test : $(OBJECTS) $(TESTS)

obj: $(OBJECTS)

clean:
	rm -rfv *.o $(OBJ_DIR)/*.o 
	rm -fv bin/test/test_*

print-%:
	@echo $* = $($*)
