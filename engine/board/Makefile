INC_DIR   = ./include/
SRC_DIR   = ./src/
OBJ_DIR   = ./obj/

objects   = board.o
OBJECTS   = $(addprefix $(OBJ_DIR), $(objects))
LIBRARY   = libboard.a
EXAMPLE   = example
CFLAGS   := $(CFLAGS) -Iinclude -DNO_DEBUG

build: $(LIBRARY)

example: $(LIBRARY) example.c
	$(CC) example.c -o $(EXAMPLE) -L. -lboard

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

