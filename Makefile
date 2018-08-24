BIN=nCA
SCR=main.c 
LIB=-lncurses -lpanel

$(BIN): $(SCR)
	$(CC) -o $@ $< $(LIB)
