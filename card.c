#include <stdio.h>
#include <stdlib.h>
#include "err.h"
#include "card.h"

/*
 * prints the given card
 * params:  card - an array of integers representing the card
 */
void print_card(Card card) {
    printf("print:\tcard %c:%d:%d,%d,%d,%d\n", (char)card[COLOR],
            card[POINTS], card[PU], card[BR], card[YE],  card[RE]);
}

/*
 * prints the given deck, for testing purposes
 * params:  deck - deck to visualize
 *          numCards - number of cards in deck
 */
void print_deck(Deck deck, int numCards) {
    printf("print:\tdeck of %d cards\n", numCards);
    for(int i = 0; i < numCards; i++) {
        print_card(deck[i]);
    }
}


/*
 * increases size of deck and stores the new card
 * params:  stack - struct containing relevant stack information
 *          color - color of card
 *          points - amount of points card is worth
 *          purple, brown, yellow, red - respective token values
 * returns: ERR if malloc fails (stack will still end with E_CARDR,
 *          OK otherwise
 */
Error add_card(Stack* stack, char color, int points, 
        int purple, int brown, int yellow, int red) {
    if(stack->numCards) {
        stack->deck = (Deck)realloc(stack->deck, 
                sizeof(Card) * (stack->numCards + 1));
    } else {
        stack->deck = (Deck)malloc(sizeof(Card));
    }

    if(!stack->deck) {
        return ERR;
    }

#ifdef VERBOSE
    printf("alloc:\tcard[%d]\n", stack->numCards);
    printf("saving:\tcard[%d] %c:%d:%d,%d,%d,%d\n", stack->numCards,
            color, points, purple, brown, yellow, red);
#endif
    
    stack->deck[stack->numCards] = (Card)malloc(sizeof(int) * CARD_SIZE + 1);
    if(!stack->deck[stack->numCards]) {
        return ERR;
    }
    stack->deck[stack->numCards][COLOR] = (int)color;
    stack->deck[stack->numCards][POINTS] = points;
    stack->deck[stack->numCards][PU] = purple;
    stack->deck[stack->numCards][BR] = brown;
    stack->deck[stack->numCards][YE] = yellow;
    stack->deck[stack->numCards][RE] = red;

#ifdef TEST 
    printf("saved:\tcard[%d] %c:%d:%d,%d,%d,%d\n", stack->numCards,
            (char)stack->deck[stack->numCards][COLOR], 
            stack->deck[stack->numCards][POINTS],
            stack->deck[stack->numCards][PU], 
            stack->deck[stack->numCards][BR],
            stack->deck[stack->numCards][YE],
            stack->deck[stack->numCards][RE]);
#endif

    stack->numCards++;
    return OK;
}

/*
 * frees memory used by the deck
 * params:  deck - array of cards to free
 *          numCards - number of cards in deck
 */
void shred_deck(Deck deck, int numCards) {
#ifdef TEST
    printf("shredding deck of %d cards\n", numCards);
#endif

    for(int i = 0; i < numCards; i++) {
        free(deck[i]);

#ifdef VERBOSE
        printf("free:\tcard %d\n", i);
#endif

    }
    free(deck);

#ifdef VERBOSE
    printf("free:\tdeck\n");
#endif
}

