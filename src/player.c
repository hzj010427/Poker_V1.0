#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "constants.h"
#include "server.h"

/****************************************************************************** Functions *******************************************************************************/

void initPlayer(Player* player, char* name, int initialChips, int socket, int isRobot, int seat) {
    strncpy(player->name, name, NAME_LENGTH - 1);
    player->name[NAME_LENGTH - 1] = '\0';  /* Ensure null termination */
    player->points = initialChips;
    player->state = PLAYER_WAITING;
    player->previousState = PLAYER_WAITING;
    player->socket = socket;
    player->isRobot = isRobot;
    player->seat = seat;
    player->betAmount = 0;
    player->raiseAmount = 0;
    player->currentBet = 0;

    for (int i = 0; i < 2; i++) {
        player->hand[i].suit = 0;
        player->hand[i].rank = 0;
    }
}


char* makeBet(Game* game, Player* player, int betAmount) {
    /* Check if the player has enough points to make the bet */
    if (player->points < betAmount) {
        return "FAIL! You don't have enough points to make this bet.\n";
    }

    /* Check if the bet amount is greater than minbet */
    if (betAmount < game->minBet) {
        return "FAIL! Bet amount must be greater than the minimum bet.\n";
    }

    /* set the current bet to the bet amount if it is greater than the current bet */
    if (betAmount > game->currentBet) {
        game->currentBet = betAmount;
        player->points -= betAmount;
        player->currentBet = betAmount;
        game->pot += betAmount;
        player->state = PLAYER_BET;
        return NULL;
    }
    else {
        return "FAIL! Bet amount must be greater than the current bet.\n";
    }

    /* Check if the player is all in */
    if (player->points == 0) {
        player->state = PLAYER_ALL_IN;
    }
}

char* callBet(Game* game, Player* player) {

    /* big blind can't call if no one has raised */
    if (player->currentBet == game->currentBet) {
        return "FAIL! can't call if no one has raised.\n";
    }
    else if (player->points >= game->currentBet) {
        int bet = game->currentBet - player->currentBet;
        player->points -= bet;
        game->pot += bet;
        player->currentBet = game->currentBet;
        player->state = PLAYER_CALLED;
        return NULL;
    }
    else {
        return "FAIL! Player does not have enough points to call.\n";
    }

    /* Check if the player is all in */
    if (player->points == 0) {
        player->state = PLAYER_ALL_IN;
    }
}

char* raiseBet(Game* game, Player* player, int raiseAmount) {

    /* Check if the player has enough points to make the raise */
    int additionalBet = raiseAmount - player->currentBet;

    if (player->points < additionalBet) {
        return "FAIL! You don't have enough points to make this raise.\n";
    }

    if (raiseAmount <= game->currentBet) {
        return "FAIL! Raise amount must be greater than the current bet.\n";
    }

    /* Increase the current bet of the game to the raise amount */
    game->currentBet = raiseAmount;
    
    /* Deduct the additional bet from the player's points */
    player->points -= additionalBet;

    /* Increase the pot by the additional bet */
    game->pot += additionalBet;

    /* Update the player's current bet to the raise amount */
    player->currentBet = game->currentBet;

    /* Check if the player is all in */
    if (player->points == 0) {
        player->state = PLAYER_ALL_IN;
    }

    return NULL;
}


char* allIn(Game* game, Player* player) {
    if (player->points > game->currentBet) {
        game->currentBet = player->points;
    }
    game->pot += player->points;
    player->currentBet += player->points;
    player->points = 0;
    player->state = PLAYER_ALL_IN;
    return NULL;
}

char* fold(Game* game, Player* player) {
    player->state = PLAYER_FOLDED;
    return NULL;
}

char* check(Game* game, Player* player) {
    if (game->currentBet == player->currentBet) {
        player->state = PLAYER_CHECKED;
        return NULL; 
    } else {
        return "FAIL! Cannot check, there is an outstanding bet.\n";
    }
}
