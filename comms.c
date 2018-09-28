#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "err.h"
#include "card.h"
#include "comms.h"
#include "common.h"
#include "signalHandler.h"

/*
 * adds token information to string
 * params:  output - string to concatenate to
 *          card - struct containing information to convert
 */
void add_token_info(char* output, Card card) {
    for(int i = PURPLE; i < CARD_SIZE; i++) {
        concat(output, to_string(card[i]));
        if(i < CARD_SIZE - 1) {
            strcat(output, ",");
        }
    }
}

/*
 * encodes a message and saves it into the given output
 * params:  msg - struct containing message details
 *          output - string containing encoded message
 * returns: NULL if invalid contents,
 *          string containing encoded message otherwise
 */
char* encode_hub(Msg* msg) {
    char* output = (char*)malloc(sizeof(char) * LINE_BUFF);
    switch(msg->type) {
        case EOG:
            strcpy(output, "eog");
            break;
        case DOWHAT:
            strcpy(output, "dowhat");
            break;
        case TOKENS:
            strcpy(output, "tokens");
            concat(output, to_string(msg->tokens));
            break;
        case NEWCARD:
            strcpy(output, "newcard");
            concat(output, card_to_string(msg->info));
            break;
        case PURCHASED:
            strcpy(output, "purchased"); 
            charcat(output, strlen("purchased"), msg->player);
            strcat(output, ":");
            concat(output, to_string(msg->card));
            strcat(output, ":");
            add_token_info(output, msg->info);
            strcat(output, ",");   
            concat(output, to_string(msg->wild));
            break;
        case WILD:
            strcpy(output, "wild");
            charcat(output, strlen("wild"), msg->player);
            break;
        case TOOK:
            strcpy(output, "took");
            charcat(output, strlen("took"), msg->player);
            strcat(output, ":");
            add_token_info(output, msg->info);
            break;
        default:
            free(output);
#ifdef TEST
            fprintf(stderr, "invalid message\n");
#endif
            return NULL;
    }

    return output;
}

/*
 * encodes a message and saves it into the given output
 * params:  msg - struct containing message details
 *          output - string containing encoded message
 * returns: NULL if invalid contents,
 *          string containing encoded message otherwise
 */
char* encode_player(Msg* msg) {
    char* output = (char*)malloc(sizeof(char) * LINE_BUFF);
    switch(msg->type) {
        case PURCHASE:
            strcpy(output, "purchase");
            concat(output, to_string(msg->card));
            strcat(output, ":");
            for(int i = PURPLE; i < CARD_SIZE; i++) {
                concat(output, to_string(msg->info[i]));
                strcat(output, ",");   
            }
            concat(output, to_string(msg->wild));
            break;
        case TAKE:
            strcpy(output, "take");
            for(int i = 0; i < TOKEN_SIZE; i++) {
                concat(output, to_string(msg->info[i + 2]));
                if(i < TOKEN_SIZE - 1) {
                    strcat(output, ",");
                }
            }
            break;
        case WILD:
            strcpy(output, "wild");
            break;
        default:
            break;
    }

    return output;
}

/*
 * decodes a message from the hub and saves it to the given message struct
 * params:  msg - struct to save message details to
 *          input - string containing message to decode
 * returns: the type of message received,
 *          ERR if invalid contents
 */
Comm decode_hub_msg(Msg* msg, char* input) {
    if(!input) {
        return ERR;
    }
    char player, color;
    char end = '\0';
    int tokens, points, purple, brown, yellow, red, wild, card;
    if(strcmp(input, "eog") == 0) {
        msg->type = EOG;
    } else if(strcmp(input, "dowhat") == OK) {
        msg->type = DOWHAT;
    } else if(sscanf(input, "wild%c%c", &player, &end) == 1 && !end) {
        msg->type = WILD;
        msg->player = player;
    } else if(sscanf(input, "tokens%d%c", &tokens, &end) == 1 && !end) {
        msg->type = TOKENS;
        msg->tokens = tokens;
    } else {
        if(sscanf(input, "newcard%c:%d:%d,%d,%d,%d,%c", &color, &points, 
                &purple, &brown, &yellow, &red, &end) == 6 && !end) {
            msg->type = NEWCARD;
            save_info(msg->info, color, points, purple, brown, yellow, red);
        } else if(sscanf(input, "purchased%c:%d:%d,%d,%d,%d,%d%c", &player, 
                &card, &purple, &brown, &yellow, &red, &wild, &end) == 7 && 
                !end) {
            msg->type = PURCHASED;
            msg->player = player;
            msg->card = card;
            save_info(msg->info, (char)0, 0, purple, brown, yellow, red);
            msg->wild = wild;
        } else if(sscanf(input, "took%c:%d,%d,%d,%d%c", &player, 
                &purple, &brown, &yellow, &red, &end) == 5 && !end) {
            msg->type = TOOK;
            msg->player = player;
            save_info(msg->info, (char)0, 0, purple, brown, yellow, red);
        } else {
#ifdef TEST
            fprintf(stderr, "invalid message: %s\n", input);
#endif
            free(input);
            return ERR;
        }
    }
    
#ifdef VERBOSE 
    fprintf(stderr, "decode:[%d] hub=%s\n", msg->type, input);
#endif

    free(input);
    return msg->type;
}

/*
 * decodes a message from a player and saves it to the given message struct
 * params:  msg - struct to save message details to
 *          input - string containing message to decode
 * returns: the type of message received,
 *          ERR if invalid contents
 */
Comm decode_player_msg(Msg* msg, char* input) {
    int card, purple, brown, yellow, red, wild;
    char end = '\0';

#ifdef TEST
    fprintf(stderr, "decode:\tplayer=%s\n", input);
#endif

    if(strcmp(input, "wild") == OK) {
        msg->type = WILD;
    } else if(sscanf(input, "purchase%d:%d,%d,%d,%d,%d%c", &card, 
            &purple, &brown, &yellow, &red, &wild, &end) == 6 && !end) {
        msg->type = PURCHASE;
        save_info(msg->info, (char)0, 0, purple, brown, yellow, red);
        msg->wild = wild;
        msg->card = card;
    } else if(sscanf(input, "take%d,%d,%d,%d%c", 
            &purple, &brown, &yellow, &red, &end) == 4 && !end) {
        msg->type = TAKE;
        save_info(msg->info, (char)0, 0, purple, brown, yellow, red);
    } else {
#ifdef TEST
        fprintf(stderr, "invalid message: %s\n", input);
#endif
        free(input);
        return ERR;
    }

    free(input);
    return msg->type;
}
