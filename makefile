CC = gcc
DEFAULT := -Wall -Wextra -pedantic -std=c99
BUILD := test
flags.test := -DTEST -g $(DEFAULT)
flags.verbose := -DTEST -DVERBOSE -g $(DEFAULT)
flags.release := $(DEFAULT) -Werror
FLAGS := $(flags.$(BUILD))
OBJ = err.o card.o common.o comms.o playerCommon.o
#token.o 

all: aus shen ban ed 
	@echo BUILD=$(BUILD)

aus: $(OBJ) 
	$(CC) -D_POSIX_C_SOURCE $(FLAGS) $(OBJ) austerity.c -o austerity

ban: $(OBJ)
	$(CC) $(FLAGS) $(OBJ) banzai.c -o banzai

ed: $(OBJ)
	$(CC) $(FLAGS) $(OBJ) ed.c -o ed

shen: $(OBJ)
	$(CC) $(FLAGS) $(OBJ) shenzi.c -o shenzi

try: 
	valgrind --leak-check=full ./austerity 1 1 deck2 ./shenzi ./shenzi

test:
	$(CC) $(DEFAULT) test.c -o test

%.o: %.c
	$(CC) $(FLAGS) -c -o $@ $<

.PHONY: all

clean:
	rm -f *.o austerity banzai ed shenzi
