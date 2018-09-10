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
            card[POINTS], card[PU],  card[BR], card[YE],  card[RE]);
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
 * params:  game - struct containing relevant game information
 *          color - color of card
 *          points - amount of points card is worth
 *          purple, brown, yellow, red - respective token values
 * returns: ERR if malloc fails (game will still end with E_CARDR,
 *          OK otherwise
 */
Error add_card(Game* game, char color, int points, 
        int purple, int brown, int yellow, int red) {
    if(game->numCards) {
        game->deck = (Deck)realloc(game->deck, 
                sizeof(Card) * (game->numCards + 1));
    } else {
        game->deck = (Deck)malloc(sizeof(Card));
    }

    if(!game->deck) {
        return ERR;
    }

#ifdef VERBOSE
    printf("alloc:\tcard[%d]\n", game->numCards);
    printf("saving:\tcard[%d] %c:%d:%d,%d,%d,%d\n", game->numCards,
            color, points, purple, brown, yellow, red);
#endif
    
    game->deck[game->numCards] = (Card)malloc(sizeof(int) * CARD_SIZE + 1);
    if(!game->deck[game->numCards]) {
        return ERR;
    }
    game->deck[game->numCards][COLOR] = (int)color;
    game->deck[game->numCards][POINTS] = points;
    game->deck[game->numCards][PU] = purple;
    game->deck[game->numCards][BR] = brown;
    game->deck[game->numCards][YE] = yellow;
    game->deck[game->numCards][RE] = red;

#ifdef TEST 
    printf("saved:\tcard[%d] %c:%d:%d,%d,%d,%d\n", game->numCards,
            (char)game->deck[game->numCards][COLOR], 
            game->deck[game->numCards][POINTS],
            game->deck[game->numCards][PU], 
            game->deck[game->numCards][BR],
            game->deck[game->numCards][YE],
            game->deck[game->numCards][RE]);
#endif

    game->numCards++;
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

