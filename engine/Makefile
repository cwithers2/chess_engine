INC_DIR   := include/
SRC_DIR   := src/
OBJ_DIR   := obj/
BIN_DIR   := bin/

HEADERS    = debug.h getline_noblock.h test.h
OBJECTS    = getline_noblock.o
GENERATED  = 
SCRIPTS    = 

LIBRARY    = #libengine.a
TEST_MAIN  = src/test.c
TEST_PROG  = ./test_suite
BUILD_MAIN = src/main.c
BUILD_PROG = ./flipflop

CFLAGS    := $(CFLAGS) -I$(INC_DIR)
CXXFLAGS  := $(CXXFLAGS) -I$(INC_DIR)

C_OBJ     := -c
CXX_OBJ   := -c

C_LIBS    := -lstdc++ -lpthread
################################################################################
headers    = $(addprefix $(INC_DIR), $(HEADERS))
objects    = $(addprefix $(OBJ_DIR), $(OBJECTS))
scripts    = $(addprefix $(BIN_DIR), $(SCRIPTS))

build: generate $(LIBRARY) $(BUILD_PROG)

test: generate $(TEST_PROG) RUN_TEST

generate:
	$(foreach script, $(scripts), $(script))

$(LIBRARY): $(objects)
	$(AR) $(ARFLAGS) $@ $^

$(BUILD_PROG): $(objects) $(BUILD_MAIN)
	$(CC) $(CFLAGS) $^ $(C_LIBS) -o $@

$(TEST_PROG): $(objects) $(TEST_MAIN)
	$(CC) $(CFLAGS) $^ $(C_LIBS) -o $@

RUN_TEST:
	$(TEST_PROG)

#c code
$(OBJ_DIR)%.o: $(SRC_DIR)%.c $(headers)
	@mkdir -p $(OBJ_DIR)
	$(CC) $(C_OBJ) $(CFLAGS) $< -o $@

#c++ code
$(OBJ_DIR)%.o: $(SRC_DIR)%.cpp $(headers)
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXX_OBJ) $(CXXFLAGS) $< -o $@

clean:
	$(RM) $(LIBRARY) $(TEST_PROG) $(BUILD_PROG)
	$(RM) $(GENERATE)
	$(RM) $(OBJECTS)
	$(RM) -R $(OBJ_DIR)

