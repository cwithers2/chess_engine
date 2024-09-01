INC_DIR   = ./include/
SRC_DIR   = ./src/
OBJ_DIR   = ./obj/

FLEX_DIR  = ./flex/
BISON_DIR = ./bison/

objects   = fen_parse.o fen_lex.o
OBJECTS   = $(addprefix $(OBJ_DIR), $(objects))
LIBRARY   = libfen.a
GENERATED = $(SRC_DIR)fen_parse.c $(SRC_DIR)fen_lex.c
EXAMPLE   = example

CFLAGS   := $(CFLAGS) -Iinclude -DNO_DEBUG

build: $(LIBRARY)

example: $(LIBRARY) example.c
	$(CC) example.c $(CFLAGS) -o $(EXAMPLE) -L. -lfen

$(LIBRARY): $(OBJECTS)
	$(AR) $(ARFLAGS) $@ $^

$(SRC_DIR)fen_parse.c: $(BISON_DIR)fen_parse.y
	bison --defines=$(INC_DIR)fen_parse.h \
	      --name-prefix=fen \
	      --output=$(SRC_DIR)fen_parse.c \
	      $<

$(SRC_DIR)fen_lex.c: $(FLEX_DIR)fen_lex.l
	flex --outfile=$@ \
	     --prefix=fen \
	     --header-file=$(INC_DIR)fen_lex.h \
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



