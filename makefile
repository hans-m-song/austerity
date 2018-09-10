CC = gcc
DEFAULT := -Wall -Werror -pedantic -std=c99
BUILD := test
flags.test := -DTEST -g $(DEFAULT)
flags.verbose := -DTEST -DVERBOSE -g $(DEFAULT)
flags.release := $(DEFAULT)
FLAGS := $(flags.$(BUILD))
OBJ = err.o comms.o common.o card.o playerCommon.o
#token.o 

all: aus shen ban ed 
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

test: 
	valgrind --leak-check=full ./austerity 1 1 deck2 ./shenzi ./shenzi

e:
	$(CC) $(DEFAULT) test.c -o test

%.o: %.c
	$(CC) $(FLAGS) -c -o $@ $<

.PHONY: all

clean:
	rm -f *.o austerity banzai ed shenzi
