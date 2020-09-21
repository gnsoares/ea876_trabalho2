
CC=gcc
FLAGS=-I./ -lfreeimage -lpthread -lm
DIR=src
SRC=$(DIR)/main.c $(DIR)/imageprocessing.c
EXE=main

PYTHON=python3
FIGURE_GEN=$(DIR)/analyze.py
PICTURE_PATH=$(filter-out $@, $(MAKECMDGOALS))

all:
	$(CC) -o$(EXE) $(SRC) $(FLAGS)

test: all
	$(PYTHON) $(FIGURE_GEN) ./$(EXE) $(PICTURE_PATH)

clean:
	$(RM) ./$(EXE)
	$(RM) ./*.o
	$(RM) $(DIR)/*.o

%:
	@: