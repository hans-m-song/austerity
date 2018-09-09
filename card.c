#include <stdio.h>
#include <stdlib.h>
#include "err.h"
#include "card.h"

/*
 * prints the given deck, for testing purposes
 * params:  deck - deck to visualize
 *          numCards - number of cards in deck
 */
void print_deck(Deck deck, int numCards) {
    for(int i = 0; i < numCards; i++) {
        printf("deck[%d] = %c:%d:%d,%d,%d,%d\n", i,
                (char)deck[numCards][COLOR], deck[numCards][POINTS],
                deck[numCards][P], deck[numCards][B],
                deck[numCards][Y], deck[numCards][R]);
        
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
Err add_card(Game* game, char color, int points, 
        int purple, int brown, int yellow, int red) {
    if(game->numCards) {
        game->deck = (Deck)realloc(game->deck, 
                sizeof(Deck) * (game->numCards + 1));

#ifdef TEST
        printf("realloc:\tdeck, %d\n cards", game->numCards + 1);
#endif

    } else {
        game->deck = (Deck)malloc(sizeof(Deck));
    }

#ifdef TEST
    printf("alloc:\tcard %d\n", game->numCards);
#endif

    if(!game->deck) {
        return ERR;
    }
    
    game->deck[game->numCards] = (Card)malloc(sizeof(int) * CARD_SIZE);
    if(!game->deck[game->numCards]) {
        return ERR;
    }
    game->deck[game->numCards][COLOR] = (int)color;
    game->deck[game->numCards][POINTS] = points;
    game->deck[game->numCards][P] = purple;
    game->deck[game->numCards][B] = brown;
    game->deck[game->numCards][Y] = yellow;
    game->deck[game->numCards][R] = red;

    return OK;
}

/*
 * frees memory used by the deck
 * params:  deck - array of cards to free
 *          numCards - number of cards in deck
 */
void shred_deck(Deck deck, int numCards) {
#ifdef TEST
    printf("shredding:\t%d cards\n", numCards);
#endif

    for(int i = 0; i < numCards; i++) {
        free(deck[i]);

#ifdef TEST
        printf("free:\tcard %d\n", i);
#endif

    }
    free(deck);

#ifdef TEST
    printf("free:\tdeck\n");
#endif
}

