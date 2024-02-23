#configuration
INC_DIR := ./include
SRC_DIR := ./src
OBJ_DIR := ./obj

#directories
INCLUDES = -I$(INC_DIR)

CC      := g++
CFLAGS  := $(CFLAGS) $(INCLUDES) 

RM      := $(RM)

#files
headers = $(INC_DIR)/getline_noblock.h
sources = $(SRC_DIR)/getline_noblock.c
objects = $(OBJ_DIR)/getline_noblock.o

build: $(objects)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(headers)
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

clean:
	$(RM) $(OBJ_DIR)/*
