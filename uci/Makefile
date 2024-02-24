#configuration
INC_DIR := ./include
SRC_DIR := ./src
OBJ_DIR := ./obj

#directories
GETLINE  = ../getline_noblock/include
UTIL     = ../util
LOCALE   = locale
INCLUDES = -I$(INC_DIR) -I$(GETLINE) -I$(UTIL) -I$(LOCALE)

CC      := g++
CFLAGS  := $(CFLAGS) $(INCLUDES) 

RM      := $(RM)

#files
headers  = $(INC_DIR)/uci.h $(GETLINE)/getline_noblock.h $(UTIL)/debug.h $(LOCALE)/src/commands.h $(LOCALE)/src/responses.h
sources  = $(SRC_DIR)/uci.c
objects  = $(OBJ_DIR)/uci.o


build: $(objects)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(headers)
	@mkdir -p $(OBJ_DIR)
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

clean:
	$(RM) -R $(OBJ_DIR)
