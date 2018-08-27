BIN=nCA
SCR=main.c 
LIB=-lncurses -lpanel -lm
CFLAGS=-g

$(BIN): $(SCR)
	$(CC) -o $@ $< $(LIB) $(CFLAGS)
