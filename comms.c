#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "err.h"
#include "card.h"
#include "comms.h"
#include "common.h"

/*
 * TODO send_msg takes the message and pipes it through the given fd
 * params:  pipe - fd to send message to
 *          msg - message to send
 * returns: E_PIPECLOSE, E_PROTOCOL, ERR, OK
 */
int send_msg(Msg* msg, int destination);

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
            concat(output, to_string(msg->player));
            strcat(output, ":");
            concat(output, to_string(msg->card));
            strcat(output, ":");
            for(int i = PURPLE; i < CARD_SIZE; i++) {
                concat(output, to_string(msg->info[i]));
                strcat(output, ",");   
            }
            concat(output, to_string(msg->wild));
            break;
        case WILD:
            strcpy(output, "wild");
            concat(output, to_string(msg->player));
            break;
        default:
            free(output);
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
            for(int i = PURPLE; i < CARD_SIZE; i++) {
                concat(output, to_string(msg->info[i]));
                if(i < CARD_SIZE - 1) {
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
                &card, &purple, &brown, &yellow, &red, &wild, &end) == 6 && 
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
            printf("invalid:\thub=%s\n", input);
#endif
            free(input);
            return ERR;
        }
    }
    
#ifdef TEST
    printf("decode:[%d] hub=%s\n", msg->type, input);
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
    char end;

#ifdef TEST
    printf("decode:\tplayer=%s\n", input);
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
        free(input);
        return ERR;
    }

    free(input);
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
