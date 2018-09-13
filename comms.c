#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "err.h"
#include "card.h"
#include "comms.h"
#include "common.h"

/*
 * takes the message and pipes it through the given fd
 * params:  pipe - fd to send message to
 *          msg - message to send
 * returns: E_PIPECLOSE, E_PROTOCOL, ERR, OK
 */
int send_msg(Msg* msg, int destination);

/*
 * encodes a message and saves it into the given output
 * params:  msg - struct containing message details
 *          output - string containing encoded message
 * returns: ERR if invalid contents,
 *          OK otherwise
 */
int encode(Msg* msg, char* output);

/*
 * decodes a message from the hub and saves it to the given message struct
 * params:  msg - struct to save message details to
 *          input - string containing message to decode
 * returns: the type of message received,
 *          ERR if invalid contents
 */
Comm decode_hub_msg(Msg* msg, char* input) {
    char player, color, end;
    int tokens, points, purple, brown, yellow, red, wild, card;
    
#ifdef TEST
    printf("decode:\thub=%s\n", input);
#endif

    if(strcmp(input, "eog") == 0) {
        msg->type = EOG;
    } else if(strcmp(input, "dowhat") == OK) {
        msg->type = DOWHAT;
    } else if(sscanf(input, "wild%c%c", &player, &end) == 1 && !end) {
        // TODO check validity
        msg->type = WILD;
        msg->player = player;
    } else if(sscanf(input, "tokens%d%c", &tokens, &end) == 1 && !end) {
        msg->type = TOKENS;
        msg->tokens = tokens;
    } else {
        msg->info = (Card)malloc(sizeof(int) * CARD_SIZE);
        if(sscanf(input, "newcard%c:%d:%d,%d,%d,%d,%c", &color, &points, 
                &purple, &brown, &yellow, &red, &end) == 6 && !end) {
            msg->type = NEWCARD;
            new_card(&msg->info, color, points, purple, brown, yellow, red);
        } else if(sscanf(input, "purchased%c:%d:%d,%d,%d,%d,%d%c", &player, 
                &card, &purple, &brown, &yellow, &red, &wild, &end) == 6 && 
                !end) {
            msg->type = PURCHASED;
            msg->player = player;
            msg->card = card;
            new_card(&msg->info, (char)0, 0, purple, brown, yellow, red);
            msg->wild = wild;
        } else if(sscanf(input, "took%c:%d,%d,%d,%d%c", &player, 
                &purple, &brown, &yellow, &red, &end) == 5 && !end) {
            msg->type = TOOK;
            msg->player = player;
            new_card(&msg->info, (char)0, 0, purple, brown, yellow, red);
        } else {
            return ERR;
        }
    }

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
    char end;

#ifdef TEST
    printf("decode:\tplayer=%s\n", input);
#endif

    if(strcmp(input, "wild") == OK) {
        msg->type = WILD;
    } else if(sscanf(input, "purchase%d:%d,%d,%d,%d,%d%c", &card, 
            &purple, &brown, &yellow, &red, &wild, &end) == 6 && !end) {
        msg->type = PURCHASE;
        new_card(&msg->info, (char)0, 0, purple, brown, yellow, red);
        msg->wild = wild;
        msg->card = card;
    } else if(sscanf(input, "take%d,%d,%d,%d%c", 
            &purple, &brown, &yellow, &red, &end) == 4 && !end) {
        msg->type = TAKE;
        new_card(&msg->info, (char)0, 0, purple, brown, yellow, red);
    } else {
        return ERR;
    }

    return msg->type;
}

/*
 * prints relevant information to stderr
 * params:  type - message type
 *          winners - list of winners to print, normally null
 */
void player_status(Comm type, char* winners) {
    switch(type) {
        case EOG:
            fprintf(stderr, "Game over. Winners are %s\n", winners);
            break;
        case DOWHAT:
            fprintf(stderr, "Received dowhat");
            break;
        default:
#ifdef TEST
            fprintf(stderr, "got invalid comm type %d\n", type);
#endif
            break;
    }
}
