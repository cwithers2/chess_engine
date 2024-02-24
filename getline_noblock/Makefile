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
HEADERS = $(INC_DIR)/getline_noblock.h
OBJECTS = $(OBJ_DIR)/getline_noblock.o

build: $(OBJECTS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(HEADERS)
	@mkdir -p $(OBJ_DIR)
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	$(RM) -R $(OBJ_DIR)
