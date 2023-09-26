#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "constants.h"
#include "server.h"

/****************************************************************************** Functions *******************************************************************************/

void initGame(Game *game, Request *request) {
    game->numPlayers = 0;
    game->totalPlayers = request->totalPlayers;
    game->totalRobots = request->robotCount;
    game->totalRoundsLeft = request->roundNumber;
    game->initialChips = request->initialChips;
    game->deck = createDeck();
    game->currentBet = (request->smallBlind) * 2;
    game->minBet = request->smallBlind;
    game->pot = 0;
    game->state = GAME_WAITING_FOR_PLAYERS;
    game->stage = PRE_FLOP;
    game->currentPlayerIndex = 0;  
    game->smallBlindIndex = 0;
    game->version = 0;

    for (int i = 0; i < 5; i++) {
        game->communityCards[i].suit = 0;
        game->communityCards[i].rank = 0;
    }

    /* Initialize winnerSeatNumbers array and numWinners */
    for (int i = 0; i < MAX_PLAYERS; i++) {
        game->winnerSeatNumbers[i] = -1;
    }
    game->numWinners = 0;
}

Player* addPlayer(Game *game, Request *request, int socket) {
    Player *player; 

    if (game->numPlayers < game->totalPlayers && (findPlayerBySeat(game, request->seatNumber) == NULL)) {
        player = &(game->players[game->numPlayers]);
        initPlayer(player, request->playerName, game->initialChips, socket, 0, request->seatNumber);
        game->numPlayers++;
        return player;
    }
    else {
        return NULL;
    }
}


/* update needed */
Player* findPlayerBySeat(Game *game, int seat) {
    Player *player;

    for (int i = 0; i < game->totalPlayers; i++) {
        player = &(game->players[i]);
        if (player->seat == seat) {
            return player;
        }
    }
    return NULL;
}

void checkWinner(Game* game, Player* winner[MAX_PLAYERS]) {
    int maxScore = -1;
    Player* topPlayer = NULL;
    int numberOfPlayersStillInGame = 0;
    int score;
    int winnersCount = 0; 
    int allIn = 1;

    /* Initialize all winners to NULL */
    for (int i = 0; i < MAX_PLAYERS; i++) {
        winner[i] = NULL;
    }

    /* check for the winner based on card value */
    for (int i = 0; i < game->totalPlayers; i++) {

        Player *player = &game->players[i];
    
        /* Only consider players who are not folded and either have points or are all-in */
        if (player->state != PLAYER_FOLDED && (player->points > 0 || player->state == PLAYER_ALL_IN)) {
            
            score = evaluateCard(game->players[i].hand, game->communityCards);

            if (score > maxScore) {
                maxScore = score;
                topPlayer = &game->players[i];
            }
            numberOfPlayersStillInGame++;
        }
    }

    /* if there is only one player left in the game, he is the winner */
    if (numberOfPlayersStillInGame == 1) {
        winnersCount = 0;
        winner[winnersCount] = topPlayer;
    }

    /* Add a check to see if all active players are all-in */
    for (int i = 0; i < game->totalPlayers; i++) {
        if (game->players[i].state != PLAYER_ALL_IN && game->players[i].state != PLAYER_FOLDED) {
            allIn = 0; 
            break;
        }
    }

    /* if we reach the post river stage, or all players are all-in, add all players with max score to the winner array */
    if (game->stage == POST_RIVER || allIn) {
        for (int i = 0; i < game->totalPlayers; i++) {
            if (game->players[i].state != PLAYER_FOLDED && evaluateCard(game->players[i].hand, game->communityCards) == maxScore) {
                winner[winnersCount] = &game->players[i];
                winnersCount++;
            }
        }
    }
}

void postBlinds(Game *game) {
    int smallBlindAmount = game->minBet;
    int bigBlindAmount = game->minBet * 2;

    /* post small blind */
    Player *smallBlindPlayer = &(game->players[game->smallBlindIndex]);
    smallBlindPlayer->points -= smallBlindAmount;
    smallBlindPlayer->currentBet = smallBlindAmount;
    game->pot += smallBlindAmount;

    /* post big blind */
    int bigBlindIndex = (game->smallBlindIndex + 1) % game->totalPlayers;
    while (game->players[bigBlindIndex].points < 0) {
        bigBlindIndex = (bigBlindIndex + 1) % game->totalPlayers;
    }

    Player *bigBlindPlayer = &(game->players[bigBlindIndex]);
    bigBlindPlayer->points -= bigBlindAmount;
    bigBlindPlayer->currentBet = bigBlindAmount;
    game->pot += bigBlindAmount;

}

void dealCommunityCards(Game *game) {

    /* shuffle the deck */
    shuffleDeck(&(game->deck));

    /* deal 5 community cards */
    for (int i = 0; i < 5; i++) {
        game->communityCards[i] = drawCard(&(game->deck));
    }
}

void dealHands(Game *game, Player *player) { 
    
    /* deal 2 cards to each player */
    for (int i = 0; i < 2; i++) {
        player->hand[i] = drawCard(&(game->deck));
    }
}

void roundOver(Game* game, Player* winners[MAX_PLAYERS]) {
    /* Reset states of all players */
    for (int i = 0; i < game->totalPlayers; i++) {
        game->players[i].state = PLAYER_WAITING;
        game->players[i].previousState = PLAYER_WAITING;
        game->players[i].betAmount = 0;
        game->players[i].raiseAmount = 0;
        game->players[i].currentBet = 0;
        for (int j = 0; j < 2; j++) {
            game->players[i].hand[j].rank = 0;
            game->players[i].hand[j].suit = 0;
        }
    }

    /* Calculate winners and add their seat numbers to winnerSeatNumbers array */
    while (winners[game->numWinners] != NULL) {
        game->winnerSeatNumbers[game->numWinners] = winners[game->numWinners]->seat;
        game->numWinners++;
    }

    int winningPortion = game->pot / (game->numWinners);

    for (int i = 0; i < (game->numWinners); i++) {
        winners[i]->points += winningPortion;
    }

    /* decrement the number of rounds left */
    game->totalRoundsLeft--;

    /* Reset game state variables */
    game->pot = 0;
    game->currentBet = (game->minBet) * 2;

    /* Check if the game should end prematurely */
    if (checkPrematureEnd(game)) {
        game->state = GAME_ENDED;
    }
    else if (game->totalRoundsLeft == 0) {
        game->state = GAME_ENDED;
    }
    else {
        /* update blind indexes for next round */
        do {
            game->smallBlindIndex = (game->smallBlindIndex + 1) % game->totalPlayers;
        } while (game->players[game->smallBlindIndex].points <= 0);
        
        if (game->totalPlayers == 2) {
            game->currentPlayerIndex = game->smallBlindIndex;
        }   
        else {
            /* set the current player to the player after the big blind */
            game->currentPlayerIndex = (game->smallBlindIndex + 2) % game->totalPlayers;

            /* Skip players who have no money */
            while(game->players[game->currentPlayerIndex].points <= 0) {
                game->currentPlayerIndex = (game->currentPlayerIndex + 1) % game->totalPlayers;
            }
        } 

        dealCommunityCards(game);
        postBlinds(game);
        game->stage = PRE_FLOP;
    }
}

void goToNextStage(Game* game) {

    /* set the stage to the next stage */
    game->stage = (game->stage + 1) % 5;
    
    /* reset the current bet */
    game->currentBet = 0;

    game->currentPlayerIndex = game->smallBlindIndex;

    /* Skip players who are all-in or folded */
    while (game->players[game->currentPlayerIndex].state == PLAYER_ALL_IN || 
           game->players[game->currentPlayerIndex].state == PLAYER_FOLDED || 
           game->players[game->currentPlayerIndex].points <= 0) {
        game->currentPlayerIndex = (game->currentPlayerIndex + 1) % game->totalPlayers;
    }

    /* Set all players who are not all-in or folded to waiting state */
    for (int i = 0; i < game->totalPlayers; i++) {
        game->players[i].betAmount = 0;
        game->players[i].raiseAmount = 0;
        game->players[i].currentBet = 0;
        if (game->players[i].state != PLAYER_ALL_IN && game->players[i].state != PLAYER_FOLDED) {
            game->players[i].state = PLAYER_WAITING;
            game->players[i].previousState = PLAYER_WAITING;
        }
    }
}

int isReadyForNextStage(Game *game) {
    for (int i = 0; i < game->totalPlayers; i++) {
        Player *player = &game->players[i];

        /* If the player is still in the game (not folded or all-in) */
        if (player->state != PLAYER_FOLDED && player->state != PLAYER_ALL_IN) {

            /* Check if the player has acted */
            if (player->state == PLAYER_WAITING) {
                return 0;
            }

            /* Check if the player has matched the current bet */
            if (player->currentBet != game->currentBet) {
                return 0;
            }
        }
    }

    /* If we reach here, all conditions are satisfied to move to the next stage */
    return 1;
}

int evaluateCard(Card *hand, Card *communityCards) {

    /* 1. Combine the hand and communityCards into one array */
    int numCards = 7;

    Card *cards = malloc(sizeof(Card) * numCards);

    for (int i = 0; i < 2; i++) {
        cards[i] = hand[i];
    }

    for (int i = 0; i < 5; i++) {
        cards[i + 2] = communityCards[i];
    }

    /* 2. Sort the cards array (from lowest to highest) */
    qsort(cards, numCards, sizeof(Card), compareCards);

    /* 3. Evaluate the hand type */
    int score, highCard1, highCard2, highCard3, highCard4, highCard5, pairRank, trioRank, quadRank, highPairRank, lowPairRank = 0;

    if (isRoyalFlush(cards, numCards)) {

        /* only one royal flush */
        score = 10000000;
    }
    else if (isStraightFlush(cards, numCards)) {
        score = 9000000;
        highCard1 = findHighCardInStraightFlush(cards, numCards);
        score += highCard1;
    }
    else if (isFourOfAKind(cards, numCards)) {
        score = 8000000;
        quadRank = findRankOfFourOfAKind(cards, numCards);
        highCard1 = findHighCardOutsideQuads(cards, numCards);
        score += quadRank * 10 + highCard1;
    }
    else if (isFullHouse(cards, numCards)) {
        score = 7000000;
        trioRank = findRankOfTrioInFullHouse(cards, numCards);
        pairRank = findRankOfPairInFullHouse(cards, numCards);
        score += trioRank * 10 + pairRank; 
    }
    else if (isFlush(cards, numCards)) {
        score = 6000000;
        highCard1 = findHighCard1InFlush(cards, numCards);
        highCard2 = findHighCard2InFlush(cards, numCards);
        highCard3 = findHighCard3InFlush(cards, numCards);
        highCard4 = findHighCard4InFlush(cards, numCards);
        highCard5 = findHighCard5InFlush(cards, numCards);
        score += highCard1 * 10000 + highCard2 * 1000 + highCard3 * 100 + highCard4 * 10 + highCard5;
    }
    else if (isStraight(cards, numCards)) {
        score = 5000000;
        highCard1 = findHighCardInStraight(cards, numCards);
        score += highCard1;
    }
    else if (isThreeOfAKind(cards, numCards)) {
        score = 4000000;
        trioRank = findRankOfThreeOfAKind(cards, numCards);
        highCard1 = findHighCard1OutsideTrio(cards, numCards);
        highCard2 = findHighCard2OutsideTrio(cards, numCards);
        score += trioRank * 100 + highCard1 * 10 + highCard2;
    }
    else if (isTwoPair(cards, numCards)) {
        score = 3000000;
        highPairRank = findRankOfHighPair(cards, numCards);
        lowPairRank = findRankOfLowPair(cards, numCards);
        highCard1 = findHighCardOutsidePairs(cards, numCards);
        score += highPairRank * 100 + lowPairRank * 10 + highCard1;
    }
    else if (isPair(cards, numCards)) {
        score = 2000000;
        pairRank = findRankOfPair(cards, numCards);
        highCard1 = findHighCard1OutsidePair(cards, numCards);
        highCard2 = findHighCard2OutsidePair(cards, numCards);
        highCard3 = findHighCard3OutsidePair(cards, numCards);
        score += pairRank * 1000 + highCard1 * 100 + highCard2 * 10 + highCard3;
    }

    /* High Card */
    else {
        score = 1000000;
        highCard1 = cards[numCards - 1].rank;
        highCard2 = cards[numCards - 2].rank;
        highCard3 = cards[numCards - 3].rank;
        highCard4 = cards[numCards - 4].rank;
        highCard5 = cards[numCards - 5].rank;
        score += highCard1 * 10000 + highCard2 * 1000 + highCard3 * 100 + highCard4 * 10 + highCard5;
    }

    free(cards);

    /* 4. Return the score of the hand */
    return score;
}

int handlePlayerAction(Game *game, Player *requestingPlayer, char *SendBuf, int DataSocketFD) {
    char* errorMessage = NULL;

    /* switch on the player state */
    switch(requestingPlayer->state) {
        case PLAYER_BET:
            errorMessage = makeBet(game, requestingPlayer, requestingPlayer->betAmount);
            if (errorMessage == NULL) {
                sprintf(SendBuf, "SUCCESS! Player %s bet %d.\n", requestingPlayer->name, requestingPlayer->betAmount);
            }
            break;
        case PLAYER_CALLED:
            errorMessage = callBet(game, requestingPlayer);
            if (errorMessage == NULL) {
                sprintf(SendBuf, "SUCCESS! Player %s called.\n", requestingPlayer->name);
            }
            break;
        case PLAYER_RAISED:
            errorMessage = raiseBet(game, requestingPlayer, requestingPlayer->raiseAmount);
            if (errorMessage == NULL) {
                sprintf(SendBuf, "SUCCESS! Player %s raised %d.\n", requestingPlayer->name, requestingPlayer->raiseAmount);
            }
            break;
        case PLAYER_CHECKED:
            errorMessage = check(game, requestingPlayer);
            if (errorMessage == NULL) {
                sprintf(SendBuf, "SUCCESS! Player %s checked.\n", requestingPlayer->name);
            }
            break;
        case PLAYER_ALL_IN:
            errorMessage = allIn(game, requestingPlayer);
            if (errorMessage == NULL) {
                sprintf(SendBuf, "SUCCESS! Player %s went all in.\n", requestingPlayer->name);
            }
            break;
        case PLAYER_FOLDED:
            errorMessage = fold(game, requestingPlayer);
            if (errorMessage == NULL) {
                sprintf(SendBuf, "SUCCESS! Player %s folded.\n", requestingPlayer->name);
            }
            break;
        default:
            errorMessage = "SUCCESS! Invalid player state.\n";
            break;
    }

    /* if there was an error notify the client, otherwise update the current player index */
    if (errorMessage != NULL) {
        /* reset the player state to the previous state */
        requestingPlayer->state = requestingPlayer->previousState;

        /* send the error message to the client */
        sprintf(SendBuf, errorMessage);
        strcat(SendBuf, "STILL YOUR TURN");
        sendMessage2Client(DataSocketFD, SendBuf);
        return 0;
    }
    else {
        if (requestingPlayer->state != PLAYER_FOLDED && requestingPlayer->state != PLAYER_ALL_IN) {
            /* set the previous state to the current state */
            requestingPlayer->previousState = requestingPlayer->state;
        }

        /* send the result to the client */
        sendMessage2Client(DataSocketFD, SendBuf);
        return 1;
    }
}

char *gameToString(Game *game, Player *requestingPlayer) {
    static char gameStatus[4096];
    char playerStatus[256];
    char communityCardStatus[256];
    char PlayerCardStatus[128];
    char winnerSeatsStatus[256];

    memset(gameStatus, 0, sizeof(gameStatus));

    /* game status */
    sprintf(gameStatus, "GameStage: %d\nGameState: %d\nPot: %d\nCurrentBet: %d\nCurrentPlayerIndex: %d\nMinBet: %d\nVersion: %d\nTotalPlayers: %d\nSmallBlindIndex: %d\nTotalRoundsLeft: %d\nNumWinners: %d\n",
            game->stage, game->state, game->pot, game->currentBet, game->currentPlayerIndex, game->minBet, game->version, game->totalPlayers, game->smallBlindIndex, game->totalRoundsLeft, game->numWinners);

    /* winners' seat numbers */
    sprintf(winnerSeatsStatus, "WinnerSeatNumbers: ");
    for(int i = 0; i < MAX_PLAYERS; i++){
        sprintf(winnerSeatsStatus + strlen(winnerSeatsStatus), "%d, ", game->winnerSeatNumbers[i]);
    }
    strcat(gameStatus, winnerSeatsStatus);
    strcat(gameStatus, "\n");

    /* community cards status */
    sprintf(communityCardStatus, "CommunityCards: ");
    for(int i = 0; i < 5; i++){
        sprintf(communityCardStatus + strlen(communityCardStatus), "%d %d, ", game->communityCards[i].rank, game->communityCards[i].suit);
    }
    strcat(gameStatus, communityCardStatus);
    strcat(gameStatus, "\n");

    /* player status */
    /* First print the requesting player's status */
    sprintf(playerStatus, "Player: %s\nSeat: %d\nPoints: %d\nIsRobot: %d\nBetAmount: %d\nRaiseAmount: %d\nCurrentBet: %d\nPlayerState: %d\n",
            requestingPlayer->name, requestingPlayer->seat, requestingPlayer->points, requestingPlayer->isRobot, requestingPlayer->betAmount, requestingPlayer->raiseAmount,
            requestingPlayer->currentBet, requestingPlayer->state);
    strcat(gameStatus, playerStatus);

    /* player cards status */
    sprintf(PlayerCardStatus, "PlayerCards: %d %d, %d %d\n", 
            requestingPlayer->hand[0].rank, requestingPlayer->hand[0].suit, requestingPlayer->hand[1].rank, requestingPlayer->hand[1].suit);
    strcat(gameStatus, PlayerCardStatus);

    /* Then print the other players' status */
    for(int i = 0; i < game->totalPlayers; i++) {
        if(&game->players[i] != requestingPlayer) {
            sprintf(playerStatus, "Player: %s\nSeat: %d\nPoints: %d\nIsRobot: %d\nBetAmount: %d\nRaiseAmount: %d\nCurrentBet: %d\nPlayerState: %d\n",
                    game->players[i].name, game->players[i].seat, game->players[i].points, game->players[i].isRobot, game->players[i].betAmount, game->players[i].raiseAmount,
                    game->players[i].currentBet, game->players[i].state);
            strcat(gameStatus, playerStatus);
        }
    }

    return gameStatus;
}

void addRobot2Game(Game *game) {
    Player *robot;
    int seat;
    int seatOccupied[MAX_PLAYERS];

    /* Initialize all seats as unoccupied */
    for (int i = 0; i < MAX_PLAYERS; i++) {
        seatOccupied[i] = 0;
    }

    /* Mark seats of existing players as occupied */
    for (int i = 0; i < game->numPlayers; i++) {
        seatOccupied[game->players[i].seat - 1] = 1;  /* Subtract 1 because seat numbers start at 1 */
    }

    /* Add robots to the game */
    for (int i = 0; i < game->totalRobots; i++) {
        robot = &game->players[game->numPlayers + i];

        /* Select an unoccupied seat */
        do {
            seat = rand() % MAX_PLAYERS;
        } while (seatOccupied[seat]);

        /* Mark the chosen seat as occupied */
        seatOccupied[seat] = 1;

        /* Initialize the robot player */
        snprintf(robot->name, NAME_LENGTH, "Robot%d", i+1);
        robot->seat = seat + 1;  /* Add 1 because seat numbers start at 1 */
        robot->points = game->initialChips;
        robot->socket = -1;
        robot->isRobot = 1;
        robot->betAmount = 0;
        robot->raiseAmount = 0;
        robot->currentBet = 0;
        robot->hand[0].rank = 0;
        robot->hand[0].suit = 0;
        robot->hand[1].rank = 0;
        robot->hand[1].suit = 0;
        robot->state = PLAYER_WAITING;
        robot->previousState = PLAYER_WAITING;
    }
}

void aiLogic(Game *game, Player *aiPlayer) {
    /* Determine the strength of the AI's hand */
    int handStrength = evaluateCard(aiPlayer->hand, game->communityCards);

    /* AI strategy based on the strength of hand and current points */
    if (handStrength > 5000000) { /* If the hand is very good */
        if (game->currentBet > aiPlayer->points) { /* if current bet is more than AI's points, go all-in */
            allIn(game, aiPlayer);
        } else if (game->currentBet < aiPlayer->points) { /* If AI has more points than current bet, raise the bet */
            raiseBet(game, aiPlayer, aiPlayer->points); /* Tries to raise to the AI's total points */
        }
    } else if (handStrength > 2000000) { /* If the hand is good */
        if (game->currentBet > aiPlayer->points) { /* if current bet is more than AI's points, go all-in */
            allIn(game, aiPlayer);
        } else if (game->currentBet < aiPlayer->points) { /* If AI has more points than current bet, make a bet or call */
            if (game->currentBet == 0) { /* If no bet has been made, make a bet */
                makeBet(game, aiPlayer, aiPlayer->points / 2); /* Bets half of AI's points */
            } else if (aiPlayer->currentBet < game->currentBet) { /* If AI has not matched the current bet, call */
                callBet(game, aiPlayer);
            } else{
                fold(game, aiPlayer);
            }
        }
    } else { /* If the hand is not good, check if possible, otherwise fold */
        if (game->currentBet > aiPlayer->points) { /* if current bet is more than AI's points, fold */
            fold(game, aiPlayer);
        } else if (game->currentBet < aiPlayer->points) { /* If AI has more points than current bet, check or fold */
            if (game->currentBet == 0) { /* If no bet has been made, check */
                check(game, aiPlayer);
            } else { /* If a bet has been made, fold */
                fold(game, aiPlayer);
            }
        }
    }
}

int checkPrematureEnd(Game *game) {
    int activePlayers = 0;

    for (int i = 0; i < game->totalPlayers; i++) {
        Player *player = &game->players[i];
        
        if (player->points > 0) {
            activePlayers++;
        }
    }

    return activePlayers == 1;
}

