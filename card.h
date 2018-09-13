#ifndef CARD_H
#define CARD_H

#include "err.h"

// number of fields in a card
#define CARD_SIZE 6

// card fields
#define COLOR 0     // color
#define POINTS 1    // points
#define PU 2        // purple
#define BR 3        // brown
#define YE 4        // yellow
#define RE 5        // red

typedef int* Card;
typedef Card* Deck;

typedef struct {
    int numCards;
    Deck deck;
} Stack;

void print_card(Card card);

void print_deck(Deck deck, int numCards);

Error add_card(Stack* stack, char color, int points,
        int purple, int brown, int yellow, int red);

void shred_deck(Deck deck, int numCards);

void new_card(Card* card, char color, int points, 
        int purple, int brown, int yellow, int red);

#endif
