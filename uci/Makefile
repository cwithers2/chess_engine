#configuration
INC_DIR := ./include
SRC_DIR := ./src
OBJ_DIR := ./obj

#directories
GETLINE  = ../getline_noblock/include
UTIL     = ../util
LOCALE   = locale
INCLUDES = -I$(INC_DIR) -I$(GETLINE) -I$(UTIL) -I$(LOCALE)

CC      := gcc
CFLAGS  := $(CFLAGS) $(INCLUDES)

RM      := $(RM)

#files
HEADERS  = $(INC_DIR)/uci.h $(GETLINE)/getline_noblock.h $(UTIL)/debug.h $(LOCALE)/src/commands.h $(LOCALE)/src/responses.h
OBJECTS  = $(OBJ_DIR)/uci.o

TARGET   = libuci.a

build: $(TARGET)

$(TARGET): $(OBJECTS)
	ar rcs $(TARGET) $(OBJECTS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(HEADERS)
	@mkdir -p $(OBJ_DIR)
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	$(RM) $(TARGET)
	$(RM) -R $(OBJ_DIR)
