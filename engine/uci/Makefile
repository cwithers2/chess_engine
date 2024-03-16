INC_DIR   = ./include/
SRC_DIR   = ./src/
OBJ_DIR   = ./obj/

FLEX_DIR  = ./flex/
BISON_DIR = ./bison/

objects   = command_parse.o command_lex.o respond_lex.o uci.o uci_node.o
OBJECTS   = $(addprefix $(OBJ_DIR), $(objects))
LIBRARY   = libuci.a
GENERATED = $(SRC_DIR)command_parse.c $(SRC_DIR)command_lex.c $(SRC_DIR)respond_lex.c $(INC_DIR)command_parse.h $(INC_DIR)command_lex.h $(INC_DIR)respond_lex.h
EXAMPLE   = example

CFLAGS   := $(CFLAGS) -Iinclude

build: $(LIBRARY)

example: $(LIBRARY) example.c
	$(CC) example.c -o $(EXAMPLE) -L. -luci

$(LIBRARY): $(OBJECTS)
	$(AR) $(ARFLAGS) $@ $^

$(SRC_DIR)command_parse.c: $(BISON_DIR)command_parse.y
	bison --defines=$(INC_DIR)command_parse.h \
	      --name-prefix=command \
	      --output=$(SRC_DIR)command_parse.c \
	      $<

$(SRC_DIR)command_lex.c: $(FLEX_DIR)command_lex.l
	flex --outfile=$@ \
	     --prefix=command \
	     --header-file=$(INC_DIR)command_lex.h \
	     $<

$(SRC_DIR)respond_lex.c: $(FLEX_DIR)respond_lex.l
	flex --outfile=$@ \
	     --prefix=respond \
	     --header-file=$(INC_DIR)respond_lex.h \
	     $<

$(OBJ_DIR)%.o: $(SRC_DIR)%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	$(RM) $(GENERATED)
	$(RM) $(OBJECTS)
	$(RM) -R $(OBJ_DIR)
	$(RM) $(LIBRARY)
	$(RM) $(EXAMPLE)

