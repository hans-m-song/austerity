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
            char* tokens = to_string(msg->tokens);
            strcat(output, tokens);
            free(tokens);
            break;
        case NEWCARD:
            strcpy(output, "newcard");
            char* card = card_to_string(msg->info);
            strcat(output, card);
            free(card);
            break;
        case PURCHASED:
            output[0] = msg->player;
            output[1] = '\0';
            char* temp = to_string(msg->card);
            strcat(output, temp);
            free(temp);
            for(int i = PURPLE; i < CARD_SIZE; i++) {
                char* temp = to_string(msg->info[i]);
                strcat(output, temp);
                free(temp);
            }
            break;
        case WILD:
            strcpy(output, "wild");
            char player[2] = {msg->player, '\0'};
            strcat(output, player);
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
char* encode_player() { //Msg* msg) {
    return "test";
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
        printf("got wild\n");
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
