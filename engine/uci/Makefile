INC_DIR   = ./include/
SRC_DIR   = ./src/
OBJ_DIR   = ./obj/

FLEX_DIR  = ./flex/
BISON_DIR = ./bison/

objects   = fen_parse.o fen_lex.o command_parse.o command_lex.o respond_lex.o uci.o uci_node.o
OBJECTS   = $(addprefix $(OBJ_DIR), $(objects))
LIBRARY   = libuci.a
GENERATED = $(SRC_DIR)command_parse.c $(SRC_DIR)command_lex.c $(SRC_DIR)respond_lex.c $(INC_DIR)command_parse.h $(INC_DIR)command_lex.h $(INC_DIR)respond_lex.h $(SRC_DIR)fen_parse.c $(INC_DIR)fen_parse.h $(INC_DIR)fen_lex.h $(SRC_DIR)fen_lex.c
EXAMPLE   = example

CFLAGS   := $(CFLAGS) -Iinclude -DNO_DEBUG

build: $(LIBRARY)

example: $(LIBRARY) example.c
	$(CC) example.c -o $(EXAMPLE) -L. -luci

$(LIBRARY): $(OBJECTS)
	$(AR) $(ARFLAGS) $@ $^

$(SRC_DIR)%.c: $(FLEX_DIR)%.l
	flex --outfile=$@ \
	     --prefix=$(word 1, $(subst _, ,$(notdir $<))) \
	     --header-file=$(INC_DIR)$(notdir $(basename $<)).h \
	     $<

$(SRC_DIR)%.c: $(BISON_DIR)%.y
	bison --defines=$(INC_DIR)$(notdir $(basename $<)).h \
	      --name-prefix=$(word 1, $(subst _, ,$(notdir $<))) \
	      --output=$@ \
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

