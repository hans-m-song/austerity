CC = gcc
DEFAULT := -Wall -Werror -pedantic -std=c99
BUILD := test
flags.test := -DTEST -g $(DEFAULT)
flags.verbose := -DTEST -DVERBOSE -g $(DEFAULT)
flags.release := $(DEFAULT)
FLAGS := $(flags.$(BUILD))
OBJ = err.o common.o card.o
POBJ = playerCommon.o
#comms.o token.o 

all: aus shen ban ed 
	@echo BUILD=$(BUILD)
#ban ed shen

aus: $(OBJ) 
	$(CC) $(FLAGS) $(OBJ) austerity.c -o austerity

ban: $(OBJ)
	$(CC) $(FLAGS) $(OBJ) $(POBJ) banzai.c -o banzai

ed: $(OBJ)
	$(CC) $(FLAGS) $(OBJ) $(POBJ)  ed.c -o ed

shen: $(OBJ)
	$(CC) $(FLAGS) $(OBJ) $(POBJ)  shenzi.c -o shenzi
	

%.o: %.c
	$(CC) $(FLAGS) -c -o $@ $<

.PHONY: all

clean:
	rm -f *.o aus
