BIN=nCA
LIB=-lncurses -lpanel -lm
CFLAGS=-g

SRC := src
OBJ := obj

SOURCES := $(wildcard $(SRC)/*.c)
OBJECTS := $(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(SOURCES))

all: $(OBJECTS)
	$(CC) $^ -o $(BIN) $(LIB) $(CFLAGS)

$(OBJ)/%.o: $(SRC)/%.c
	$(CC) -I$(SRC) -c $< -o $@ $(CFLAGS)
