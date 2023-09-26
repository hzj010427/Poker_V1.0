#ifndef CLIENT_H
#define CLIENT_H

#include "constants.h"

/****************************************************************************** Card Structure *******************************************************************************/

typedef enum {
    HEARTS, //红桃
    DIAMONDS,   //方块
    CLUBS,  //梅花
    SPADES  //黑桃
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

/****************************************************************************** Player Structure *******************************************************************************/

typedef struct {
    char name[NAME_LENGTH];
    int seat;
    int points;
    int isRobot;
    int betAmount;
    int raiseAmount;
    int currentBet;
    int state;
    Card hand[2];
} PlayerInfo;

typedef struct {
    char name[NAME_LENGTH];
    int seat;
    int points;
    int isRobot;
    int betAmount;
    int raiseAmount;
    int currentBet;
    int state;
} PublicPlayerInfo;

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
    int version;
    int stage;
    int state;
    int pot;
    int currentBet;
    int currentPlayerIndex;
    int minBet;
    int totalPlayers;
    int smallBlindIndex;
    int totalRoundsLeft;
    Card communityCards[5]; 
    PlayerInfo player;
    PublicPlayerInfo publicPlayers[MAX_PLAYERS - 1];
    int winnerSeatNumbers[MAX_PLAYERS]; /* Array to store seat numbers of winners */
    int numWinners; /* Number of winners */
} GameInfo;

extern GameInfo gameinfo;
extern int needInput;
extern int isLatest;

/****************************************************************************** Request Functions *******************************************************************************/

void FatalError(const char *ErrorMsg);

char *Talk2Server(const char *Message, char *RecvBuf);

char *ReceiveMessageFromServer(const char *command);

void ParseGameInfo(GameInfo *gameinfo, char *serverResponse, PlayerInfo *player);

void PrintGameInfo(GameInfo *gameinfo);

void printCard(Card card);

void printWinners(const GameInfo* gameinfo);

void GtkDrawStart(int argc, char *argv[]);

void ReceiveGameInfo();

#endif

/****************************************************************************** GUI Functions *******************************************************************************/
/* GUI的函数声明放在这里 */
/* GUI的函数实现需要新建一个名为gui.c的文件并把函数实现放入gui.c的文件中 */