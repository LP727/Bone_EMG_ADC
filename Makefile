# Edited as needed
#
program := bone_emg_adc

SRC_DIR	:= src
OBJ_DIR	:= obj
INC_DIR := include

INC := $(wildcard $(INC_DIR)/*.h)
SRC := $(wildcard $(SRC_DIR)/*.c)
OBJ := $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

CC :=gcc
CFLAGS := -O3 -Wall -lpruio -pthread -I$(INC_DIR)

.PHONY: all run clean

all: $(program)

$(program): $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@ 

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR) $(INC)
	$(CC) $(CFLAGS) -c  $< -o $@

$(OBJ_DIR):
	mkdir $@

run: $(program)
	./$(program)

clean:
	rm -r $(OBJ_DIR) $(program)
