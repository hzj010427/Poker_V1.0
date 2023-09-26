/*
windows compile command:
gcc client.c gui.c -mms-bitfields -IC:/gtk/include/gtk-2.0 -IC:/gtk/lib/gtk-2.0/include -IC:/gtk/include/atk-1.0 -IC:/gtk/include/cairo -IC:/gtk/include/gdk-pixbuf-2.0 -IC:/gtk/include/pango-1.0 -IC:/gtk/include/glib-2.0 -IC:/gtk/lib/glib-2.0/include -IC:/gtk/include/pixman-1 -IC:/gtk/include -IC:/gtk/include/freetype2 -IC:/gtk/include/libpng14 -LC:/gtk/lib  -l"gtk-win32-2.0" -l"gdk-win32-2.0" -l"atk-1.0" -l"gio-2.0" -l"pangowin32-1.0" -lgdi32 -l"pangocairo-1.0" -l"gdk_pixbuf-2.0" -lpng14 -l"pango-1.0" -lcairo -l"gobject-2.0" -l"gmodule-2.0" -l"gthread-2.0" -l"glib-2.0" -lintl -lws2_32
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>

#include "client.h"
#include "constants.h"

/****************************************************************************** Global Variables *******************************************************************************/

const char *Program	= NULL; /* program name for descriptive diagnostics */

struct sockaddr_in ServerAddress;	/* server address we connect with */

int SocketFD; /* socket file descriptor for Internet stream socket */

GameInfo gameinfo;
int needInput;
int isLatest;

/****************************************************************************** Functions *******************************************************************************/

int main(int argc, char *argv[]) {
    int PortNo;
    struct hostent *Server;
    Program = argv[0];

    if (argc < 3) {
        fprintf(stderr, "Usage: %s hostname port\n", argv[0]);
        exit(10);
    }

    Server = gethostbyname(argv[1]);
    if (Server == NULL) {
        fprintf(stderr, "%s: no such host named '%s'\n", argv[0], argv[1]);
        exit(10);
    }
    PortNo = atoi(argv[2]);
    if (PortNo <= 2000) {
        fprintf(stderr, "%s: invalid port number %d, should be >2000\n", argv[0], PortNo);
        exit(10);
    }
    memset(&ServerAddress, 0, sizeof(ServerAddress));
    ServerAddress.sin_family = AF_INET;
    ServerAddress.sin_port = htons(PortNo);
    ServerAddress.sin_addr = *(struct in_addr*)Server->h_addr_list[0];

    /* Create a TCP socket */
    SocketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (SocketFD < 0) {
        FatalError("socket creation failed");
    }
 
    if (connect(SocketFD, (struct sockaddr*)&ServerAddress, sizeof(struct sockaddr_in)) < 0) {
        FatalError("connecting to server failed");
    }

    needInput = 0;
    isLatest = 1;

    GtkDrawStart(argc, argv);

    // close(SocketFD);

    return 0;
}

void ReceiveGameInfo() {
    char command[BUFFSIZE];
    char* serverResponse;

    /* Request the latest game status from the server */
    sprintf(command, "STATUS %d", gameinfo.version);
    serverResponse = ReceiveMessageFromServer(command);

    /* Parse the server's response to update the game status */
    if (strcmp(serverResponse, "LATEST") != 0) {
        isLatest = 0;
        printf("%s\n", serverResponse);
        ParseGameInfo(&gameinfo, serverResponse, &(gameinfo.player));

        /* If the server's response indicates it's our turn and the game has started, set needInput to 1 */
        if (strstr(serverResponse, "YOUR TURN") != NULL && (gameinfo.state == GAME_STARTED || gameinfo.state == GAME_WAITING_FOR_START) && (gameinfo.player.hand[0].rank != 0)) {
            needInput = 1;
        } 
        else {
            needInput = 0;
        }

        /* If there is a winner, print the winner */

        /****************** 这里不再是打印赢家里的内容而是要更新ui界面 ********************/
        if (gameinfo.numWinners > 0) {
            printWinners(&gameinfo);
        }
        /*******************************************************************************/

        /* Update the GUI with the latest game status */

        /****************** 这里不再是打印结构里的内容而是要更新ui界面 ********************/
        PrintGameInfo(&gameinfo);

        /*******************************************************************************/
    } else {
        isLatest = 1;
    }
}

void FatalError(		/* print error diagnostics and abort */
	const char *ErrorMsg)
{
    fputs(Program, stderr);
    fputs(": ", stderr);
    perror(ErrorMsg);
    fputs(Program, stderr);
    fputs(": Exiting!\n", stderr);
    exit(20);
} /* end of FatalError */

char *Talk2Server(const char *Message, char *RecvBuf) {
    int n;

    /* clear the receive buffer before writing */
    memset(RecvBuf, 0, BUFFSIZE);

    n = write(SocketFD, Message, strlen(Message));
    if (n < 0) {
        FatalError("writing to socket failed");
    }

    n = read(SocketFD, RecvBuf, BUFFSIZE-1);
    if (n < 0) {   
        FatalError("reading from socket failed");
    }

    if (n == 0) {
        FatalError("server closed connection");
    }

    /* Clear the rest of the buffer after reading */
    memset(RecvBuf + n, 0, BUFFSIZE - n);

    return(RecvBuf);
} /* end of Talk2Server */

char *ReceiveMessageFromServer(const char *command) {
    static char RecvBuf[BUFFSIZE];
    char *Response = Talk2Server(command, RecvBuf);
    return Response;
}

void ParseGameInfo(GameInfo *gameinfo, char *serverResponse, PlayerInfo *player) {
    int n;
    char *p = serverResponse;

    /* Parse game status */
    n = sscanf(p, "GameStage: %d\nGameState: %d\nPot: %d\nCurrentBet: %d\nCurrentPlayerIndex: %d\nMinBet: %d\nVersion: %d\nTotalPlayers: %d\nSmallBlindIndex: %d\nTotalRoundsLeft: %d\nNumWinners: %d\n",
        &(gameinfo->stage),
        &(gameinfo->state),
        &(gameinfo->pot),
        &(gameinfo->currentBet),
        &(gameinfo->currentPlayerIndex),
        &(gameinfo->minBet),
        &(gameinfo->version),
        &(gameinfo->totalPlayers),
        &(gameinfo->smallBlindIndex),
        &(gameinfo->totalRoundsLeft),
        &(gameinfo->numWinners)
    );
    
    /* Moving pointer to CommunityCards */
    for(int i = 0; i < 11; i++) {
        p = strchr(p, '\n') + 1; 
    }    

    /* Parse Winners seat number */
    n += sscanf(p, "WinnerSeatNumbers: %d, %d, %d, %d, %d, %d, \n",
        &(gameinfo->winnerSeatNumbers[0]),
        &(gameinfo->winnerSeatNumbers[1]),
        &(gameinfo->winnerSeatNumbers[2]),
        &(gameinfo->winnerSeatNumbers[3]),
        &(gameinfo->winnerSeatNumbers[4]),
        &(gameinfo->winnerSeatNumbers[5])
    );
    p = strchr(p, '\n') + 1;

    /* Parse community cards status */
    n += sscanf(p, "CommunityCards: %d %d, %d %d, %d %d, %d %d, %d %d, \n",
        &(gameinfo->communityCards[0].rank), &(gameinfo->communityCards[0].suit),
        &(gameinfo->communityCards[1].rank), &(gameinfo->communityCards[1].suit),
        &(gameinfo->communityCards[2].rank), &(gameinfo->communityCards[2].suit),
        &(gameinfo->communityCards[3].rank), &(gameinfo->communityCards[3].suit),
        &(gameinfo->communityCards[4].rank), &(gameinfo->communityCards[4].suit)
    );
    p = strchr(p, '\n') + 1;

    /* Parse the requesting player's status */
    n += sscanf(p, "Player: %s\nSeat: %d\nPoints: %d\nIsRobot: %d\nBetAmount: %d\nRaiseAmount: %d\nCurrentBet: %d\nPlayerState: %d\n",
        player->name,
        &(player->seat),
        &(player->points),
        &(player->isRobot),
        &(player->betAmount),
        &(player->raiseAmount),
        &(player->currentBet),
        &(player->state)
    );
    /* Move to the next player */
    for(int j = 0; j < 8; j++) {
        p = strchr(p, '\n') + 1;
    }

    /* Parse player cards status */
    n += sscanf(p, "PlayerCards: %d %d, %d %d\n",
        &(player->hand[0].rank),
        &(player->hand[0].suit),
        &(player->hand[1].rank),
        &(player->hand[1].suit)
    );
    p = strchr(p, '\n') + 1;

    /* Parse the other players' status */
    for(int i = 0; i < gameinfo->totalPlayers - 1; i++) {
        PublicPlayerInfo* publicPlayer = &gameinfo->publicPlayers[i];
        n += sscanf(p, "Player: %s\nSeat: %d\nPoints: %d\nIsRobot: %d\nBetAmount: %d\nRaiseAmount: %d\nCurrentBet: %d\nPlayerState: %d\n",
            publicPlayer->name,
            &(publicPlayer->seat),
            &(publicPlayer->points),
            &(publicPlayer->isRobot),
            &(publicPlayer->betAmount),
            &(publicPlayer->raiseAmount),
            &(publicPlayer->currentBet),
            &(publicPlayer->state)
        );
        /* Move to the next player */
        for(int j = 0; j < 8; j++) {
            p = strchr(p, '\n') + 1;
        }
    }

    printf("Parsed %d items\n", n);
}

void printCard(Card card) {
    char *suits[4] = {"H", "D", "C", "S"};
    char *ranks[15] = {" ", " ", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K", "A"};
    printf("|%s of %s|", ranks[card.rank], suits[card.suit]);
}

void PrintGameInfo(GameInfo *gameinfo) {
    printf("Version: %d, Stage: %d, State: %d, Pot: %d, Current Bet: %d, Player Index: %d, Min Bet: %d, Total Players: %d, Small Blind Index: %d, Total Rounds Left: %d\n",
            gameinfo->version, gameinfo->stage, gameinfo->state, gameinfo->pot, gameinfo->currentBet, gameinfo->currentPlayerIndex, gameinfo->minBet, gameinfo->totalPlayers, gameinfo->smallBlindIndex, gameinfo->totalRoundsLeft);
    printf("Community Cards: ");
    for (int i = 0; i < 5; i++) {
        printCard(gameinfo->communityCards[i]);
    }
    printf("\n");

    printf("Player Name: %s, Seat: %d, Points: %d, Is Robot: %d, Bet Amount: %d, Raise Amount: %d, Current Bet: %d, State: %d, Cards: ", 
            gameinfo->player.name, gameinfo->player.seat, gameinfo->player.points, gameinfo->player.isRobot, gameinfo->player.betAmount, gameinfo->player.raiseAmount, gameinfo->player.currentBet, gameinfo->player.state);
    for (int j = 0; j < 2; j++) {
        printCard(gameinfo->player.hand[j]);
    }
    printf("\n");

    /* Now also print information about the other players */
    for (int i = 0; i < gameinfo->totalPlayers - 1; i++) {
        PublicPlayerInfo *publicPlayer = &(gameinfo->publicPlayers[i]);
        printf("Player Name: %s, Seat: %d, Points: %d, Is Robot: %d, Bet Amount: %d, Raise Amount: %d, Current Bet: %d, State: %d\n",
            publicPlayer->name, publicPlayer->seat, publicPlayer->points, publicPlayer->isRobot, publicPlayer->betAmount, publicPlayer->raiseAmount, publicPlayer->currentBet, publicPlayer->state);
    }
}

void printWinners(const GameInfo* gameinfo) {
    printf("Round winners:\n");
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (gameinfo->winnerSeatNumbers[i] != -1) {
            if (gameinfo->winnerSeatNumbers[i] == gameinfo->player.seat) {
                printf("Winner is %s at seat number %d\n", gameinfo->player.name, gameinfo->player.seat);
            } 
            else {
                for (int j = 0; j < gameinfo->totalPlayers - 1; j++) {
                    if (gameinfo->winnerSeatNumbers[i] == gameinfo->publicPlayers[j].seat) {
                        printf("Winner is %s at seat number %d\n", gameinfo->publicPlayers[j].name, gameinfo->publicPlayers[j].seat);
                        break;
                    }
                }
            }
        } 
        else {
            /* No more winners, break the loop */
            break;
        }
    }
}







