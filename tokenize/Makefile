#configuration
INC_DIR := ./include
SRC_DIR := ./src
OBJ_DIR := ./obj

CC      := gcc
CFLAGS  := $(CFLAGS)

RM      := $(RM)

#files
HEADERS = $(INC_DIR)/tokenize.h
OBJECTS = $(OBJ_DIR)/tokenize.o

TARGET  = libtokenize.a

build: $(TARGET)

$(TARGET): $(OBJECTS)
	ar rcs $(TARGET) $(OBJECTS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(HEADERS)
	@mkdir -p $(OBJ_DIR)
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	$(RM) $(TARGET)
	$(RM) -R $(OBJ_DIR)
