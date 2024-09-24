INC_DIR   = ./include/
SRC_DIR   = ./src/
OBJ_DIR   = ./obj/

objects   = board.o magic.o
OBJECTS  ?= $(addprefix $(OBJ_DIR), $(objects))
LIBRARY   = libboard.a
EXAMPLE   = example
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

clean:
	$(RM) $(OBJECTS)
	$(RM) -R $(OBJ_DIR)
	$(RM) $(LIBRARY)
	$(RM) $(EXAMPLE)

