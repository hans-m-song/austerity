#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "err.h"
#include "card.h"
#include "common.h"
#include "comms.h"

/*
 * prints the given card
 * params:  card - an array of integers representing the card
 *          position - place in stack
 */
void print_card(Card card, int position) {
    fprintf(stderr, "Card %d:%c/%d/%d,%d,%d,%d\n", 
            position, (char)card[COLOR], card[POINTS], 
            card[PURPLE], card[BROWN], card[YELLOW], card[RED]);
}

/*
 * prints the given deck, for testing purposes
 * params:  deck - deck to visualize
 *          numCards - number of cards in deck
 */
void print_deck(Deck deck, int numCards) {
    for(int i = 0; i < numCards; i++) {
        print_card(deck[i], i);
    }
}

/*
 * announces card to stdout
 * params:  card - card to print
 */
void announce_card(Card card) {
    fprintf(stdout, "New card = Bonus %c, worth %d, costs %d,%d,%d,%d\n",
            (char)card[COLOR], card[POINTS],
            card[PURPLE], card[BROWN],
            card[YELLOW], card[RED]);
}

/*
 * a wrapper for add_card and remove_card, 
 * moves card from stack1 to stack2
 * params:  stack - struct containing deck and number of cards
 *          card - int array containing information about card
 * returns: ERR if addcard fails,
 *          OK otherwise
 */
Error move_card(Stack* source, Stack* destination, int card) {
    if(new_card(destination, source->deck[card]) != OK) {
        return ERR;
    }

    if(remove_card(source, card) != OK) {
        return ERR;
    }

    return OK;
}

/*
 * a wrapper for add_card, taking a card struct instead of explicit values
 * params:  stack - struct containing deck and number of cards
 *          card - int array containing information about card
 * returns: ERR if addcard fails,
 *          OK otherwise
 */
Error new_card(Stack* stack, Card card) {
    return add_card(stack, card[COLOR], card[POINTS], 
            card[PURPLE], card[BROWN], 
            card[YELLOW], card[RED]);
}

/*
 * increases size of deck and stores the new card
 * params:  stack - struct containing relevant stack information
 *          color - color of card
 *          points - amount of points card is worth
 *          purple, brown, yellow, red - respective token values
 * returns: ERR if malloc fails,
 *          OK otherwise
 */
Error add_card(Stack* stack, char color, int points, 
        int purple, int brown, int yellow, int red) {
    if(stack->numCards) {
        stack->deck = (Deck)realloc(stack->deck,
                sizeof(Card) * (stack->numCards + 1));
    }

#ifdef VERBOSE
    fprintf(stderr, "alloc:\tcard[%d]\n", stack->numCards);
    fprintf(stderr, "saving:\tcard[%d] %c:%d:%d,%d,%d,%d\n", stack->numCards,
            color, points, purple, brown, yellow, red);
#endif
    
    stack->deck[stack->numCards] = (Card)malloc(sizeof(int) * CARD_SIZE + 1);
    if(!stack->deck[stack->numCards]) {
        return ERR;
    }
    stack->deck[stack->numCards][COLOR] = (int)color;
    stack->deck[stack->numCards][POINTS] = points;
    stack->deck[stack->numCards][PURPLE] = purple;
    stack->deck[stack->numCards][BROWN] = brown;
    stack->deck[stack->numCards][YELLOW] = yellow;
    stack->deck[stack->numCards][RED] = red;

#ifdef VERBOSE 
    fprintf(stderr, "saved:\tcard[%d] %c:%d:%d,%d,%d,%d\n", stack->numCards,
            (char)stack->deck[stack->numCards][COLOR], 
            stack->deck[stack->numCards][POINTS],
            stack->deck[stack->numCards][PURPLE], 
            stack->deck[stack->numCards][BROWN],
            stack->deck[stack->numCards][YELLOW],
            stack->deck[stack->numCards][RED]);
#endif

    stack->numCards++;
    return OK;
}

/*
 * removes the given card from the stack
 * params:  stack - struct containing deck and numCards
 *          card - card to search and remove
 * returns: ERR if card not found,
 *          OK otherwise
 */
Error remove_card(Stack* stack, int card) {
#ifdef VERBOSE 
    fprintf(stderr, "remove:\tcard[%d]\n", card);
#endif

    if (card > stack->numCards - 1) {
        return ERR;
    }

    if(card != stack->numCards - 1) {
        for(int i = card; i < stack->numCards - 1; i++) {
            for(int j = 0; j < CARD_SIZE; j++) {
#ifdef VERBOSE 
                fprintf(stderr, "%d:%d, %d <- %d\n", i, j, 
                        stack->deck[i][j], stack->deck[i + 1][j]);
#endif
                stack->deck[i][j] = stack->deck[i + 1][j];
            }
        }
    }
    stack->numCards--;
    free(stack->deck[stack->numCards]);
    return OK;
}

/*
 * frees memory used by the deck
 * params:  deck - array of cards to free
 *          numCards - number of cards in deck
 */
void shred_deck(Deck deck, int numCards) {
#ifdef VERBOSE
    fprintf(stderr, "shredding deck of %d cards\n", numCards);
#endif
    
    for(int i = 0; i < numCards; i++) {
        free(deck[i]);
    }
    free(deck);
}

/*
 * saves the information about tokens from the message 
 * params:  card - struct to save card details to
 *          color, points, purple, brown, yellow, red - card details
 */
void save_info(Card card, char color, int points, 
        int purple, int brown, int yellow, int red) {
    card[COLOR] = (int)color;
    card[POINTS] = points;
    card[PURPLE] = purple;
    card[BROWN] = brown;
    card[YELLOW] = yellow;
    card[RED] = red;
}

/*
 * converts the given card to string format
 * params:  card - struct containing card information
 * returns: string containing card details
 */
char* card_to_string(Card card) {
    char* output = (char*)malloc(sizeof(char) * LINE_BUFF);
    char color[3] = {(char)card[0], ':', '\0'};
    strcpy(output, color);
    char* temp = to_string(card[1]);
    strcat(output, temp);
    free(temp);
    strcat(output, ":");
    for(int i = PURPLE; i < CARD_SIZE; i++) {
        char* temp = to_string(card[i]); 
        strcat(output, temp);
        free(temp);
        if(i < RED) {
            strcat(output, ",");
        }
    }

    return output;
}

/*
 * checks contents of deck and saves to memory
 * params:  deckFile - file containing deck
 *          stack - struct containing relevant stack information
 * returns: ERR if invalid contents,
 *          OK otherwise
 */
Error read_deck(FILE* deckFile, Stack* stack) {
    Error err = OK;
    char* line = (char*)malloc(sizeof(char) * LINE_BUFF);
    while(1) {
        if(fgets(line, LINE_BUFF, deckFile) == NULL) {
            if(!stack->numCards) {
                err = ERR;
            }
            break;
        }
      
        char color, end;
        int res, points, purple, brown, yellow, red;
        res = sscanf(line, "%c:%d:%d,%d,%d,%d%c", 
                &color, &points, &purple, &brown, &yellow, &red, &end);
        char* validColors = "PBYR";
        if(res != 7 || end != '\n' || strspn(&color, validColors) != 1 || 
                points < 0 || purple < 0 || brown < 0 || yellow < 0 || 
                red < 0) {
            err = ERR;
            break;
        }
        
#ifdef VERBOSE 
        fprintf(stderr, "fgets:\t%s", line);
#endif

        if(add_card(stack, color, points, 
                purple, brown, yellow, red) != OK) {
            return ERR;
        }
    }
    free(line);

#ifdef TEST
    fprintf(stderr, "got:\t%d cards\n", stack->numCards);
#endif
    
    if(stack->numCards != 0 && err != OK) {
        shred_deck(stack->deck, stack->numCards);
    }

    return err;
}

