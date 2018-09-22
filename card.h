#ifndef CARD_H
#define CARD_H

#include "err.h"

// number of fields in a card
#define CARD_SIZE 6

// card fields
#define COLOR 0
#define POINTS 1
#define PURPLE 2
#define BROWN 3
#define YELLOW 4
#define RED 5

typedef int* Card;
typedef Card* Deck;

typedef struct {
    int numCards;
    Deck deck;
} Stack;

void print_card(Card card, int position);

void print_deck(Deck deck, int numCards);

Error add_card(Stack* stack, char color, int points,
        int purple, int brown, int yellow, int red);

Error remove_card(Stack* stack, int card);

void shred_deck(Deck deck, int numCards);

void save_info(Card card, char color, int points, 
        int purple, int brown, int yellow, int red);

char* card_to_string(Card card);

Error read_deck(FILE* deckFile, Stack* stack);

#endif
