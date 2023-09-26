#ifndef SERVER_H
#define SERVER_H

#include <unistd.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <arpa/inet.h>

#include "constants.h"

/****************************************************************************** Card Structure *******************************************************************************/

typedef enum {
    HEARTS,
    DIAMONDS,
    CLUBS,
    SPADES
} Suit;

typedef enum {
    RANK_2 = 2,
    RANK_3,
    RANK_4,
    RANK_5,
    RANK_6,
    RANK_7,
    RANK_8,
    RANK_9,
    RANK_10,
    RANK_JACK,
    RANK_QUEEN,
    RANK_KING,
    RANK_ACE
} Rank;

typedef struct {
    int rank;
    int suit;
} Card;

typedef struct {
    Card cards[NUM_CARDS];
    int topCardIndex;
} Deck;

/****************************************************************************** Player Structure *******************************************************************************/

typedef enum {
    PLAYER_WAITING,
    PLAYER_BET,
    PLAYER_CALLED,
    PLAYER_RAISED,
    PLAYER_CHECKED,
    PLAYER_FOLDED,
    PLAYER_ALL_IN,
} PlayerState;

typedef struct {
    char name[NAME_LENGTH];
    int seat;
    int points;
    int socket;
    int isRobot;
    int betAmount;
    int raiseAmount;
    int currentBet;
    Card hand[2];
    PlayerState state;
    PlayerState previousState;
} Player;

/****************************************************************************** Request Structure *******************************************************************************/

typedef enum {
    REQUEST_CREATE_ROOM = 1, /* New request type for the room owner. */
    REQUEST_JOIN,
    REQUEST_GET,
    REQUEST_BET,
    REQUEST_CALL,
    REQUEST_RAISE,
    REQUEST_CHECK,
    REQUEST_ALL_IN,
    REQUEST_FOLD,
    REQUEST_STATUS, 
} RequestType;

typedef struct {
    char playerName[NAME_LENGTH]; /* The name of the player making the request. */
    RequestType type; /* The type of the request. */
    int betAmount; /* For bet and raise requests, the amount of the bet. */
    int raiseAmount; /* For raise requests, the amount of the raise. */
    int totalPlayers; /* For room creation, the total number of players (including robots). */
    int robotCount; /* For room creation, the number of robots. */
    int initialChips; /* For room creation, the initial number of chips for each player. */
    int roundNumber; /* For room creation, the round number. */
    int smallBlind; /* For room creation, the small blind amount. */
    int seatNumber; /* For join requests, the desired seat number. */
    int version; /* The version of the client. */
} Request;

/****************************************************************************** Game Structure *******************************************************************************/

typedef enum {
    PRE_FLOP,
    FLOP,
    TURN,
    RIVER,
    POST_RIVER,
} GameStage;

typedef enum {
    GAME_WAITING_FOR_PLAYERS = 1,
    GAME_WAITING_FOR_START,
    GAME_STARTED,
    GAME_ENDED,
} GameState;

typedef struct {
    Player players[MAX_PLAYERS];
    int numPlayers; /* human players */
    int totalPlayers; /* human + robot players */
    int totalRobots;
    int totalRoundsLeft;
    int initialChips;
    Card communityCards[5];
    Deck deck;
    GameStage stage; 
    GameState state;
    int pot;
    int currentBet; 
    int currentPlayerIndex; 
    int minBet;
    int smallBlindIndex;
    int version;
    int winnerSeatNumbers[MAX_PLAYERS]; /* Array to store seat numbers of winners */
    int numWinners; /* Number of winners */
} Game;

/****************************************************************************** Request Functions *******************************************************************************/

void FatalError(const char *ErrorMsg);

int MakeServerSocket(uint16_t PortNo);

int ProcessRequest(int DataSocketFD, Game *game, fd_set *ActiveFDs);

void ServerMainLoop(int ServSocketFD, Game *game, int Timeout);

void sendMessage2Client(int DataSocketFD, const char *message);

int handlePlayerAction(Game *game, Player *requestingPlayer, char *SendBuf, int DataSocketFD);

Request* parseRequest(const char *msg);

/****************************************************************************** Player Functions *******************************************************************************/

void initPlayer(Player* player, char* name, int initialChips, int socket, int isRobot, int seat);

Player* findPlayerBySocket(Game *game, int socket);

Player* findPlayerBySeat(Game *game, int seat);

char* makeBet(Game* game, Player* player, int betAmount);

char* callBet(Game* game, Player* player);

char* raiseBet(Game* game, Player* player, int raiseAmount);

char* allIn(Game* game, Player* player);

char* fold(Game* game, Player* player);

char* check(Game* game, Player* player);

/****************************************************************************** Game Functions *******************************************************************************/

void initGame(Game *game, Request *request);

void ProcessGameEvent(Game *game, int DataSocketFD);

Player* addPlayer(Game *game, Request *request, int DataSocketFD);

void checkWinner(Game *game, Player* winners[MAX_PLAYERS]);

void postBlinds(Game *game);

void dealCommunityCards(Game *game);

void dealHands(Game *game, Player *player);

void roundOver(Game* game, Player* winners[MAX_PLAYERS]);

void goToNextStage(Game *game);

int evaluateCard(Card *hand, Card *communityCards);

int isReadyForNextStage(Game *game);

char *gameToString(Game *game, Player *requestingPlayer);

void addRobot2Game(Game *game);

void aiLogic(Game *game, Player *aiPlayer);

int checkPrematureEnd(Game *game);

/****************************************************************************** Card Functions *******************************************************************************/

Card createCard(int rank, int suit);

Deck createDeck();

void shuffleDeck(Deck* deck);

Card drawCard(Deck* deck);

int compareCards(const void *a, const void *b);

int isRoyalFlush(Card *cards, int numCards);

int isStraightFlush(Card *cards, int numCards);

int isFourOfAKind(Card *cards, int numCards);

int isFullHouse(Card *cards, int numCards);

int isFlush(Card *cards, int numCards);

int isStraight(Card *cards, int numCards);

int isThreeOfAKind(Card *cards, int numCards);

int isTwoPair(Card *cards, int numCards);

int isPair(Card *cards, int numCards);

int findFlushSuit(Card *cards, int numCards);

int findRankOfPair(Card *cards, int numCards);

int findRankOfLowPair(Card *cards, int numCards);

int findRankOfHighPair(Card *cards, int numCards);

int findRankOfThreeOfAKind(Card *cards, int numCards);

int findRankOfFourOfAKind(Card *cards, int numCards);

int findHighCardInStraight(Card *cards, int numCards);

int findHighCard1InFlush(Card *cards, int numCards);

int findHighCard2InFlush(Card *cards, int numCards);

int findHighCard3InFlush(Card *cards, int numCards);

int findHighCard4InFlush(Card *cards, int numCards);

int findHighCard5InFlush(Card *cards, int numCards);

int findHighCard1OutsideTrio(Card *cards, int numCards);

int findHighCard2OutsideTrio(Card *cards, int numCards);

int findHighCardOutsidePairs(Card *cards, int numCards);

int findHighCard1OutsidePair(Card *cards, int numCards);

int findHighCard2OutsidePair(Card *cards, int numCards);

int findHighCard3OutsidePair(Card *cards, int numCards);

int findHighCardInStraightFlush(Card *cards, int numCards);

int findRankOfTrioInFullHouse(Card *cards, int numCards);

int findRankOfPairInFullHouse(Card *cards, int numCards);

int findHighCardOutsideQuads(Card *cards, int numCards);

#endif