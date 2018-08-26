BIN=nCA
SCR=main.c 
LIB=-lncurses -lpanel -lm

$(BIN): $(SCR)
	$(CC) -o $@ $< $(LIB)
