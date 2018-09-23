CC = gcc
#BUILD := test
BUILD := release
flags.test 	:= -Wall -Wextra -pedantic -std=gnu99 -g -DTEST
flags.verbose 	:= -Wall -Wextra -pedantic -std=gnu99 -g -DTEST -DVERBOSE
flags.release 	:= -Wall -Wextra -pedantic -std=gnu99 -g -Werror
FLAGS := $(flags.$(BUILD))
OBJ = err.o card.o common.o comms.o playerCommon.o signalHandler.o token.o

all: aus shen ban ed 
	@echo BUILD=$(BUILD)

aus: $(OBJ) hub.o 
	$(CC) $(FLAGS) $(OBJ) hub.o austerity.c -o austerity

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
