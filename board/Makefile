#configuration
INC_DIR := ./include
SRC_DIR := ./src
OBJ_DIR := ./obj
TST_DIR := ./test

#directories
INCLUDES = -I$(INC_DIR)

CC      := gcc
CFLAGS  := $(CFLAGS) $(INCLUDES)

RM      := $(RM)

#files
HEADERS  = $(INC_DIR)/board.h $(INC_DIR)/board_types.h
SOURCES  = $(SRC_DIR)/board.c
OBJECTS  = $(OBJ_DIR)/board.o

TARGET   = libboard.a
TEST_OUT = $(TST_DIR)/test

build: $(TARGET)
test: $(TEST_OUT)

$(TEST_OUT): $(SOURCES) ../util/test.h
	@mkdir -p $(TST_DIR)
	$(CC) $(CFLAGS) -I../util $(SRC_DIR)/test.c -o $(TEST_OUT)

$(TARGET): $(OBJECTS)
	ar rcs $(TARGET) $(OBJECTS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(HEADERS)
	@mkdir -p $(OBJ_DIR)
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	$(RM) $(TARGET)
	$(RM) -R $(OBJ_DIR)
	$(RM) -R $(TST_DIR)
