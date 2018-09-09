CC = gcc
BUILD := test
flags.test := -DTEST -std=c99 -Wall -pedantic -g
flags.verbose := -DTEST -DVERBOSE -std=c99 -Wall -pedantic -g
flags.release := -std=c99 -Wall -pedantic -g
FLAGS := $(flags.$(BUILD))
OBJ = err.o common.o
#comms.o token.o card.o player_common.o

all: aus 
	@echo BUILD=$(BUILD)
#ban ed shen

aus: $(OBJ)
	$(CC) $(FLAGS) $(OBJ) austerity.c -o austerity

ban: $(OBJ)
	$(CC) $(FLAGS) $(OBJ) banzai.c -o banzai

ed: $(OBJ)
	$(CC) $(FLAGS) $(OBJ) ed.c -o ed

shen: $(OBJ)
	$(CC) $(FLAGS) $(OBJ) shenzi.c -o shenzi
	

%.o: %.c
	$(CC) $(FLAGS) -c -o $@ $<

.PHONY: all

clean:
	rm -f *.o aus
