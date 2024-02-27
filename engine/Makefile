################################################################################
INC_DIR := include/
SRC_DIR := src/
OBJ_DIR := obj/
BIN_DIR := bin/

HEADERS  = board.h board_types.h debug.h getline_noblock.h test.h tokenize.h uci.h
OBJECTS  = board.o getline_noblock.o tokenize.o uci.o
GENERATE = include/commands.h include/responses.h src/commands.c src/responses.c
SCRIPTS  = generate_locale.py
LIBRARY  = libengine.a
PROGRAM  =
MAIN     =

CFLAGS  := $(CFLAGS) -I$(INC_DIR)
################################################################################
headers  = $(addprefix $(INC_DIR), $(HEADERS))
objects  = $(addprefix $(OBJ_DIR), $(OBJECTS))
scripts  = $(addprefix $(BIN_DIR), $(SCRIPTS))

build: generate $(LIBRARY) $(PROGRAM)

generate:
	$(foreach script, $(scripts), $(script))

$(LIBRARY): $(objects)
	$(AR) $(ARFLAGS) $@ $^

$(PROGRAM): $(objects)
	$(CC) $(CFLAGS) $(MAIN) $< -o $@

$(OBJ_DIR)%.o: $(SRC_DIR)%.c $(headers)
	@mkdir -p $(OBJ_DIR)
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	$(RM) $(LIBRARY)
	$(RM) $(PROGRAM)
	$(RM) $(GENERATE)
	$(RM) $(OBJECTS)
	$(RM) -R $(OBJ_DIR)

