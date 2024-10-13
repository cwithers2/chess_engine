INC_DIR   = ./include/
SRC_DIR   = ./src/
OBJ_DIR   = ./obj/

objects   = board.o lookup.o util.o
OBJECTS  ?= $(addprefix $(OBJ_DIR), $(objects))
LIBRARY   = libboard.a
EXAMPLE   = example
TEST      = test
CFLAGS   ?= -Iinclude


build: $(LIBRARY)

example: $(LIBRARY) example.c
	$(CC) example.c $(SRC_DIR)debug.c -o $(EXAMPLE) -L. -lboard $(CFLAGS) -DDEBUG

debug:
	OBJECTS="$(OBJECTS) $(OBJ_DIR)debug.o" \
	CFLAGS="$(CFLAGS) -DDEBUG" \
	make build

$(LIBRARY): $(OBJECTS)
	$(AR) $(ARFLAGS) $@ $^

$(OBJ_DIR)%.o: $(SRC_DIR)%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) -c $(CFLAGS) $< -o $@

test: $(LIBRARY) $(SRC_DIR)test.c
	$(CC) $(SRC_DIR)test.c -o $(TEST) -L. -lboard $(CFLAGS)

clean:
	$(RM) $(OBJECTS)
	$(RM) -R $(OBJ_DIR)
	$(RM) $(LIBRARY)
	$(RM) $(EXAMPLE)
	$(RM) $(TEST)

