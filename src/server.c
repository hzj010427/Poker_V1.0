#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <arpa/inet.h>

#include "constants.h"
#include "server.h"

/****************************************************************************** Global Variables *******************************************************************************/

const char *Program	= NULL; /* program name for descriptive diagnostics */

int Shutdown = 0; /* global shutdown flag */

int gameAction = 0; /* global flag indicating the game related action */

int gameEndVisitCount = 0; /* global counter for the number of times the client has visited the game end state */

/****************************************************************************** Functions *******************************************************************************/

int main(int argc, char *argv[]) {
    int ServSocketFD;
    int PortNo;
    Game game;

    Program = argv[0];

    if (argc < 2) {
        fprintf(stderr, "Usage: %s port\n", Program);
        exit(10);
    }
    PortNo = atoi(argv[1]);
    if (PortNo <= 2000) {
        fprintf(stderr, "%s: invalid port number %d, should be >2000\n", Program, PortNo);
        exit(10);
    }

    ServSocketFD = MakeServerSocket(PortNo);

    ServerMainLoop(ServSocketFD, &game, 250000);

    close(ServSocketFD);

    return 0;
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

int MakeServerSocket(uint16_t PortNo) {
    int ServSocketFD;
    struct sockaddr_in ServSocketName;

    /* create the socket (IPv4 TCP)*/
    ServSocketFD = socket(PF_INET, SOCK_STREAM, 0);
    if (ServSocketFD < 0)
    {   FatalError("service socket creation failed");
    }
    /* bind the socket to this server */
    ServSocketName.sin_family = AF_INET;
    ServSocketName.sin_port = htons(PortNo);
    ServSocketName.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(ServSocketFD, (struct sockaddr*)&ServSocketName,
		sizeof(ServSocketName)) < 0)
    {   FatalError("binding the server to a socket failed");
    }
    /* start listening to this socket */
    if (listen(ServSocketFD, 6) < 0)	/* max 6 clients in backlog */
    {   FatalError("listening on socket failed");
    }
    return ServSocketFD;
} /* end of MakeServerSocket */

Request* parseRequest(const char *msg) {

    /* request will be freed by the caller */
    Request* request = malloc(sizeof(Request));

    if (request == NULL) {
        printf("Error: Could not allocate memory for request.\n");
        free(request);
        return NULL;
    }

    char temp[BUFFSIZE];
    strncpy(temp, msg, sizeof(temp));

    char *token = strtok(temp, " ");
    if (token == NULL) {
        printf("Error: Invalid request format.\n");
        free(request);
        return NULL;
    }

    /* Determine the request type based on the first token. */
    if (strcmp(token, "CREAT") == 0) {
        request->type = REQUEST_CREATE_ROOM;

        /* Parse the room creation request parameters. */
        if ((token = strtok(NULL, " ")) == NULL ||
            strlen(token) >= NAME_LENGTH || 
        strncpy(request->playerName, token, NAME_LENGTH - 1) == NULL ||
            (token = strtok(NULL, " ")) == NULL ||
            strcmp(token, "SEAT") != 0 ||
            (token = strtok(NULL, " ")) == NULL ||
            (request->seatNumber = atoi(token)) < 0 ||
            (token = strtok(NULL, " ")) == NULL ||
            strcmp(token, "PLAYER") != 0 ||
            (token = strtok(NULL, " ")) == NULL ||
            /* the room must have at least 2 players and at most 6 players */
            (request->totalPlayers = atoi(token)) < MIN_PLAYERS || request->totalPlayers > MAX_PLAYERS ||
            (token = strtok(NULL, " ")) == NULL ||
            strcmp(token, "BOT") != 0 ||
            (token = strtok(NULL, " ")) == NULL ||
            (request->robotCount = atoi(token)) < 0 || request->robotCount > request->totalPlayers ||
            (token = strtok(NULL, " ")) == NULL ||
            strcmp(token, "POINT") != 0 ||
            (token = strtok(NULL, " ")) == NULL ||
            (request->initialChips = atoi(token)) < 0 ||
            (token = strtok(NULL, " ")) == NULL ||
            strcmp(token, "ROUND") != 0 ||
            (token = strtok(NULL, " ")) == NULL ||
            (request->roundNumber = atoi(token)) < 0 ||
            (token = strtok(NULL, " ")) == NULL ||
            strcmp(token, "SB") != 0 ||
            (token = strtok(NULL, " ")) == NULL ||
            (request->smallBlind = atoi(token)) < 0 || request->smallBlind > request->initialChips) {
            free(request);
            return NULL;
        }
    }
   else if (strcmp(token, "JOIN") == 0) {
        request->type = REQUEST_JOIN;

        /* Parse the join request parameters. */
        if ((token = strtok(NULL, " ")) == NULL ||
            strlen(token) >= NAME_LENGTH || 
        strncpy(request->playerName, token, NAME_LENGTH - 1) == NULL ||
            (token = strtok(NULL, " ")) == NULL ||
            strcmp(token, "SEAT") != 0 ||
            (token = strtok(NULL, " ")) == NULL ||
            (request->seatNumber = atoi(token)) < 1 || request->seatNumber > MAX_PLAYERS) {
            free(request);    
            return NULL;
        } 
    }

    else if (strcmp(token, "GET") == 0) {
        request->type = REQUEST_GET;

        /* Parse the get request parameters. */
        if ((token = strtok(NULL, " ")) == NULL ||
            strcmp(token, "SEAT") != 0 ||
            (token = strtok(NULL, " ")) == NULL ||
            (request->seatNumber = atoi(token)) < 0) {
            free(request);
            return NULL;
        }
    }
    else if (strcmp(token, "BET") == 0) {
        request->type = REQUEST_BET;

        /* Parse the bet request parameters. */
        if ((token = strtok(NULL, " ")) == NULL ||
            (request->betAmount = atoi(token)) < 0) {
            free(request);
            return NULL;
        }
    }
    else if (strcmp(token, "CALL") == 0) {
        request->type = REQUEST_CALL;
    }
    else if (strcmp(token, "RAISE") == 0) {
        request->type = REQUEST_RAISE;

        /* Parse the raise request parameters. */
        if ((token = strtok(NULL, " ")) == NULL ||
            (request->raiseAmount = atoi(token)) < 0) {
            free(request);
            return NULL;
        }
    }
    else if (strcmp(token, "CHECK") == 0) {
        request->type = REQUEST_CHECK;
    }
    else if (strcmp(token, "ALLIN") == 0) {
        request->type = REQUEST_ALL_IN;
    }
    else if (strcmp(token, "FOLD") == 0) {
        request->type = REQUEST_FOLD;
    }
    else if (strcmp(token, "STATUS") == 0) {
        request->type = REQUEST_STATUS;

        /* Parse the request version. */
        if ((token = strtok(NULL, " ")) == NULL ||
            (request->version = atoi(token)) < 0) {
            free(request);
            return NULL;
        }
    }
    else {
        free(request);
        return NULL;
    }
    
    return request;
}

Player* findPlayerBySocket(Game *game, int socket) {
    for (int i = 0; i < game->totalPlayers; i++) {
        if (game->players[i].socket == socket) {
            return &(game->players[i]);
        }
    }

    return NULL;  /* Player not found */
}

void sendMessage2Client(int DataSocketFD, const char *message) {
    int length = strlen(message);
    
    /* Send the message to the client */
    int result = write(DataSocketFD, message, length);

    if (result < 0) {
        FatalError("writing to data socket failed");
    }
}

int ProcessRequest(int DataSocketFD, Game *game, fd_set *ActiveFDs) {
    int n;
    char RecvBuf[BUFFSIZE];	/* message buffer for receiving a message */
    char SendBuf[BUFFSIZE];	/* message buffer for sending a response */
    char gameStatus[1024]; 
    Request* request;
    Player* player;
    Player *winner[MAX_PLAYERS];

    n = read(DataSocketFD, RecvBuf, sizeof(RecvBuf)-1);

    if (n < 0) 
    {   
        FatalError("reading from data socket failed");
    }

    /* The client is disconnected */
    else if (n == 0 && (game->state == GAME_STARTED || game->state == GAME_ENDED))
    {   
        player = findPlayerBySocket(game, DataSocketFD);
        close(DataSocketFD);
        FD_CLR(DataSocketFD, ActiveFDs);

        /* replaced by AI */
        player->state = PLAYER_FOLDED;
        player->socket = -1;  
        player->isRobot = 1;  
        game->version++;

        /* if the disconnected player is the current player, move to the next player */
        if (player == &(game->players[game->currentPlayerIndex])) {

            /* check if there is a winner */
            checkWinner(game, winner);  

            /* if there is a winner, end the round */
            if (winner[0] != NULL) {
                roundOver(game, winner);
                game->version++;
            }

            /* check if the game is ready to move to the next stage */
            else if (isReadyForNextStage(game)) { 
                goToNextStage(game); 
                game->version++;
            }

            /* the game is not ready to move to the next stage, so update the current player index */
            else {

                /* update the current player index and skip players who are all-in or folded */
                do {
                    game->currentPlayerIndex = (game->currentPlayerIndex + 1) % game->totalPlayers;
                } while (game->players[game->currentPlayerIndex].state == PLAYER_ALL_IN || 
                        game->players[game->currentPlayerIndex].state == PLAYER_FOLDED);
            }
        }
        return 1;
    }
    
    
    else if (n > 0) {

        RecvBuf[n] = 0;

        /* Parse the received message to determine the request type. */
        request = parseRequest(RecvBuf);

        /* If the request is invalid, notify the client. */
        if (!request) {
            sprintf(SendBuf, "Invalid request.\n");
        }
        else {

            /* process the request */
            switch (request->type) {

                /* update the game status to the client */
                case REQUEST_STATUS:

                gameAction = 0;

                if (game->state == GAME_ENDED) {
                    break;
                }
                else if (request->version == game->version) {
                    sprintf(SendBuf, "LATEST");
                } 
                else {
                    /* Game information + player turn information */
                    strcpy(gameStatus, gameToString(game, findPlayerBySocket(game, DataSocketFD)));

                    if (game->players[game->currentPlayerIndex].socket == DataSocketFD && ((game->state == GAME_STARTED) || (game->state == GAME_WAITING_FOR_START))) {
                        strcat(gameStatus, "YOUR TURN");
                    }
                    sprintf(SendBuf, gameStatus);
                }
                break;

                case REQUEST_CREATE_ROOM:

                gameAction = 0;

                    /* If the room has already been created, notify the client. */
                    if ((game->state == GAME_WAITING_FOR_PLAYERS) || (game->state == GAME_STARTED) || (game->state == GAME_ENDED) || (game->state == GAME_WAITING_FOR_START)) {
                        sprintf(SendBuf, "The room has already been created.\n");
                    }
                    else {
                        /* Create a new room, initialize the game, and add the player to the room. */
                        initGame(game, request);
                        player = addPlayer(game, request, DataSocketFD);

                        if (player) {
                            sprintf(SendBuf, "SUCCESS! Player %s has joined the game at seat %d.\n", player->name, player->seat);
                        }
                        else {
                            sprintf(SendBuf, "FAIL! Cannot add player %s at seat %d.\n", request->playerName, request->seatNumber);
                        }
                    }
                    break;

                case REQUEST_JOIN:

                gameAction = 0;

                    /* If the game has already started, notify the client. */
                    if (game->state == GAME_STARTED) {
                        sprintf(SendBuf, "FAIL! The game is full\n");
                    }
                    /* the room has not been created yet */
                    else if (game->state != GAME_WAITING_FOR_PLAYERS && game->state != GAME_ENDED && game->state != GAME_STARTED && game->state != GAME_WAITING_FOR_START) {
                        sprintf(SendBuf, "FAIL! The room has not been created yet.\n");
                    }
                    else {
                        /* Add the player to the existing room. */
                        player = addPlayer(game, request, DataSocketFD);

                        if (player) {
                            sprintf(SendBuf, "SUCCESS! Player %s has joined the game at seat %d.\n", player->name, player->seat);
                        }
                        else {
                            sprintf(SendBuf, "FAIL! Cannot add player %s at seat %d.\n", request->playerName, request->seatNumber);
                        } 
                    }
                    break;

                case REQUEST_GET:

                gameAction = 0;

                    player = findPlayerBySeat(game, request->seatNumber);

                    /* the room has not been created yet */
                    if (game->state != GAME_WAITING_FOR_PLAYERS && game->state != GAME_ENDED && game->state != GAME_STARTED && game->state != GAME_WAITING_FOR_START) {
                        sprintf(SendBuf, "FAIL! The room has not been created yet.\n");
                    }
                    else if (player) {
                        sprintf(SendBuf, "Player %s is at seat %d\n", player->name, player->seat);
                    } 
                    else {
                        sprintf(SendBuf, "No player at seat %d\n", request->seatNumber);
                    }
                    break;

                case REQUEST_BET:

                gameAction = 1;

                    /* If the game has not started yet, notify the client. */
                    if (game->state != GAME_STARTED) {
                        sprintf(SendBuf, "FAIL! The game has not yet started.\n");
                    } 
                    else {
                        player = findPlayerBySocket(game, DataSocketFD);

                        if (player == NULL) {
                            sprintf(SendBuf, "FAIL! You are not in the game.\n");
                            break;
                        }

                        player->betAmount = request->betAmount;
                        player->state = PLAYER_BET;
                    }
                    break;

                case REQUEST_CALL:

                gameAction = 1;

                    /* If the game has not started yet, notify the client. */
                    if (game->state != GAME_STARTED) {
                        sprintf(SendBuf, "FAIL! The game has not yet started.\n");
                    } 
                    else {
                        player = findPlayerBySocket(game, DataSocketFD);

                        if (player == NULL) {
                            sprintf(SendBuf, "FAIL! You are not in the game.\n");
                            break;
                        }

                        player->state = PLAYER_CALLED;
                    }
                    break;

                case REQUEST_RAISE:

                gameAction = 1;

                    /* If the game has not started yet, notify the client. */
                    if (game->state != GAME_STARTED) {
                        sprintf(SendBuf, "FAIL! The game has not yet started.\n");
                    } 
                    else {
                        player = findPlayerBySocket(game, DataSocketFD);

                        if (player == NULL) {
                            sprintf(SendBuf, "FAIL! You are not in the game.\n");
                            break;
                        }

                        player->raiseAmount = request->raiseAmount;
                        player->state = PLAYER_RAISED;
                    }
                    break;

                case REQUEST_CHECK:

                gameAction = 1;
                
                    /* If the game has not started yet, notify the client. */
                    if (game->state != GAME_STARTED) {
                        sprintf(SendBuf, "FAIL! The game has not yet started.\n");
                    } 
                    else {
                        player = findPlayerBySocket(game, DataSocketFD);

                        if (player == NULL) {
                            sprintf(SendBuf, "FAIL! You are not in the game.\n");
                            break;
                        }

                        player->state = PLAYER_CHECKED;
                    }
                    break;

                case REQUEST_ALL_IN:

                gameAction = 1;

                    /* If the game has not started yet, notify the client. */
                    if (game->state != GAME_STARTED) {
                        sprintf(SendBuf, "FAIL! The game has not yet started.\n");
                    } 
                    else {
                        player = findPlayerBySocket(game, DataSocketFD);

                        if (player == NULL) {
                            sprintf(SendBuf, "FAIL! You are not in the game.\n");
                            break;
                        }

                        player->state = PLAYER_ALL_IN;
                    }
                    break;

                case REQUEST_FOLD:

                gameAction = 1;

                    /* If the game has not started yet, notify the client. */
                    if (game->state != GAME_STARTED) {
                        sprintf(SendBuf, "FAIL! The game has not yet started.\n");
                    } 
                    else {
                        player = findPlayerBySocket(game, DataSocketFD);

                        if (player == NULL) {
                            sprintf(SendBuf, "FAIL! You are not in the game.\n");
                            break;
                        }
                        
                        player->state = PLAYER_FOLDED;
                    }
                    break;

                default:
                    gameAction = 0;
                    sprintf(SendBuf, "FAIL! Error: Invalid request type or format.\n");
                    break;  
            }
        }

        /* send the response in SendBuf to the client */
        sendMessage2Client(DataSocketFD, SendBuf);
        memset(SendBuf, 0, sizeof(SendBuf));

        /* free the request that malloced in parseRequest */
        free(request);
    }

    /* no players disconnected */
    return 0;
} 

void ProcessGameEvent(Game *game, int DataSocketFD) {
    char SendBuf[BUFFSIZE];	
    Player *requestingPlayer;
    Player *winner[MAX_PLAYERS];
    int allPlayersDealt = 1;

    /* switch on the game state */
    switch (game->state) {
        case GAME_WAITING_FOR_PLAYERS:
            
            /* check if all players have joined the game */
            if ((game->numPlayers + game->totalRobots) == game->totalPlayers) {
                game->state = GAME_WAITING_FOR_START;

                /* add AI */
                for (int i = 0; i < game->totalRobots; i++) {
                    addRobot2Game(game);
                }

                /* Bubble sort players based on their seat number */
                for (int i = 0; i < game->totalPlayers; i++) {
                    for (int j = i + 1; j < game->totalPlayers; j++) {
                        if (game->players[i].seat > game->players[j].seat) {
                            /* Swap players[i] and players[j] */
                            Player temp = game->players[i];
                            game->players[i] = game->players[j];
                            game->players[j] = temp;
                        }
                    }
                }

                dealCommunityCards(game);
                postBlinds(game);

                if (game->totalPlayers == 2) {
                    game->currentPlayerIndex = game->smallBlindIndex;
                }   
                else {
                    /* set the current player to the player after the big blind */
                    game->currentPlayerIndex = (game->smallBlindIndex + 2) % game->totalPlayers;
                } 

                /* gameinfo has changed */
                game->version++;
            }
            break;

        case GAME_WAITING_FOR_START:

            /* deal hands for all players if they haven't been dealt yet */
            for (int i = 0; i < game->totalPlayers; i++) {
                if (game->players[i].hand[0].rank == 0) {
                    dealHands(game, &game->players[i]);
                    game->version++;
                }
            }

            /* check if all players have been dealt their hands */
            allPlayersDealt = 1;
            for (int i = 0; i < game->totalPlayers; i++) {
                if (game->players[i].hand[0].rank == 0) {
                    allPlayersDealt = 0;
                    break;
                }
            }   

            if (allPlayersDealt) {
                game->state = GAME_STARTED;

                /* gameinfo has changed */
                game->version++;   
            }
            break;

        case GAME_STARTED:
            
            requestingPlayer = findPlayerBySocket(game, DataSocketFD);

            /* if a player's hand is reset, deal the hand again */
            for (int i = 0; i < game->totalPlayers; i++) {
                if (game->players[i].hand[0].rank == 0) {
                    dealHands(game, &game->players[i]);
                    game->version++;
                }
            }

            /* if current player is AI, execute AI logic */
            if (game->players[game->currentPlayerIndex].isRobot) {
                aiLogic(game, &game->players[game->currentPlayerIndex]);
                game->version++;

                /* check if there is a winner */
                checkWinner(game, winner);  

                /* if there is a winner, end the round */
                if (winner[0] != NULL) {
                    roundOver(game, winner);
                    game->version++;
                }
    
                /* check if the game is ready to move to the next stage */
                else if (isReadyForNextStage(game)) { 
                    goToNextStage(game); 
                    game->version++;
                }

                /* the game is not ready to move to the next stage, so update the current player index */
                else {

                    /* update the current player index and skip players who are all-in or folded */
                    do {
                        game->currentPlayerIndex = (game->currentPlayerIndex + 1) % game->totalPlayers;
                    } while (game->players[game->currentPlayerIndex].state == PLAYER_ALL_IN || 
                            game->players[game->currentPlayerIndex].state == PLAYER_FOLDED);
                }
            }

            /* break out of the loop if the player's action is not game related */
            if (gameAction == 0) {
                break;
            }

            /* Initialize winnerSeatNumbers array and numWinners */
            for (int i = 0; i < MAX_PLAYERS; i++) {
                game->winnerSeatNumbers[i] = -1;
            }
            game->numWinners = 0;

            /* switch on the game stage */
            switch (game->stage) {
                case PRE_FLOP:
                    if (handlePlayerAction(game, requestingPlayer, SendBuf, DataSocketFD)) {
                        game->version++;
                    }
                    else {
                        game->version++;
                        break;
                    }
                    
                    /* check if there is a winner */
                    checkWinner(game, winner);  

                    /* if there is a winner, end the round */
                    if (winner[0] != NULL) {
                        roundOver(game, winner);
                        game->version++;
                    }
        
                    /* check if the game is ready to move to the next stage */
                    else if (isReadyForNextStage(game)) { 
                        goToNextStage(game); 
                        game->version++;
                    }

                    /* the game is not ready to move to the next stage, so update the current player index */
                    else {

                        /* update the current player index and skip players who are all-in or folded */
                        do {
                            game->currentPlayerIndex = (game->currentPlayerIndex + 1) % game->totalPlayers;
                        } while (game->players[game->currentPlayerIndex].state == PLAYER_ALL_IN || 
                                game->players[game->currentPlayerIndex].state == PLAYER_FOLDED ||
                                game->players[game->currentPlayerIndex].points <= 0);
                    }
                    break;

                case FLOP:
                    if (handlePlayerAction(game, requestingPlayer, SendBuf, DataSocketFD)) {
                        game->version++;
                    }
                    else {
                        game->version++;
                        break;
                    }
                    
                    /* check if there is a winner */
                    checkWinner(game, winner);  

                    /* if there is a winner, end the round */
                    if (winner[0] != NULL) {
                        roundOver(game, winner);
                        game->version++;
                    }
        
                    /* check if the game is ready to move to the next stage */
                    else if (isReadyForNextStage(game)) { 
                        goToNextStage(game); 
                        game->version++;
                    }

                    /* the game is not ready to move to the next stage, so update the current player index */
                    else {

                        /* update the current player index and skip players who are all-in or folded */
                        do {
                            game->currentPlayerIndex = (game->currentPlayerIndex + 1) % game->totalPlayers;
                        } while (game->players[game->currentPlayerIndex].state == PLAYER_ALL_IN || 
                                game->players[game->currentPlayerIndex].state == PLAYER_FOLDED ||
                                game->players[game->currentPlayerIndex].points <= 0);
                    }
                    break;

                case TURN:
                    if (handlePlayerAction(game, requestingPlayer, SendBuf, DataSocketFD)) {
                        game->version++;
                    }
                    else {
                        game->version++;
                        break;
                    }
                    
                    /* check if there is a winner */
                    checkWinner(game, winner);  

                    /* if there is a winner, end the round */
                    if (winner[0] != NULL) {
                        roundOver(game, winner);
                        game->version++;
                    }
        
                    /* check if the game is ready to move to the next stage */
                    else if (isReadyForNextStage(game)) { 
                        goToNextStage(game); 
                        game->version++;
                    }

                    /* the game is not ready to move to the next stage, so update the current player index */
                    else {

                        /* update the current player index and skip players who are all-in or folded */
                        do {
                            game->currentPlayerIndex = (game->currentPlayerIndex + 1) % game->totalPlayers;
                        } while (game->players[game->currentPlayerIndex].state == PLAYER_ALL_IN || 
                                game->players[game->currentPlayerIndex].state == PLAYER_FOLDED ||
                                game->players[game->currentPlayerIndex].points <= 0);
                    }
                    break;

                case RIVER:
                    if (handlePlayerAction(game, requestingPlayer, SendBuf, DataSocketFD)) {
                        game->version++;
                    }
                    else {
                        game->version++;
                        break;
                    }

                    /* check if there is a winner */
                    checkWinner(game, winner);  

                    /* if there is a winner, end the round */
                    if (winner[0] != NULL) {
                        roundOver(game, winner);
                        game->version++;
                    }
        
                    /* check if the game is ready to move to the next stage */
                    else if (isReadyForNextStage(game)) { 
                        goToNextStage(game); 
                        game->version++;
                    }

                    /* the game is not ready to move to the next stage, so update the current player index */
                    else {

                        /* update the current player index and skip players who are all-in or folded */
                        do {
                            game->currentPlayerIndex = (game->currentPlayerIndex + 1) % game->totalPlayers;
                        } while (game->players[game->currentPlayerIndex].state == PLAYER_ALL_IN || 
                                game->players[game->currentPlayerIndex].state == PLAYER_FOLDED ||
                                game->players[game->currentPlayerIndex].points <= 0);
                    }
                    break;

                case POST_RIVER:

                    /* check if there is a winner */
                    checkWinner(game, winner);  

                    /* if there is a winner, end the round */
                    if (winner[0] != NULL) {
                        roundOver(game, winner);
                        game->version++;
                        sendMessage2Client(DataSocketFD, "NEXT ROUND\n");
                    }
                    else {
                        /* error */
                        FatalError("Game is in POST_RIVER stage but there is no winner.");
                    }
                    break;
            }
            break;

        case GAME_ENDED:
            /* Increase game end visit count */
            gameEndVisitCount++;

            /* Check if all human players have visited */
            if (gameEndVisitCount >= game->numPlayers) {
                Shutdown = 1;  /* Set shutdown flag to 1 */
            }

            /* Send END message to client */
            sendMessage2Client(DataSocketFD, "GAME OVER");
            printf("GAME OVER. The server is about to shut down...\n");
            break;

        default:
            break;
    }
}

void ServerMainLoop(int ServSocketFD, Game *game, int Timeout) {
    int DataSocketFD;
    socklen_t ClientLen;
    struct sockaddr_in ClientAddress;
    fd_set ActiveFDs;
    fd_set ReadFDs;
    /* struct timeval TimeVal; */
    int res, i, disconnected;

    FD_ZERO(&ActiveFDs);
    FD_SET(ServSocketFD, &ActiveFDs);

    while(!Shutdown) {

        /* copy FDs */
        ReadFDs = ActiveFDs;

        /*
        TimeVal.tv_sec  = Timeout / 1000000;
        TimeVal.tv_usec = Timeout % 1000000;
        */

        /* res is the number of ready descriptors */
        res = select(FD_SETSIZE, &ReadFDs, NULL, NULL, NULL/* &TimeVal */);

        /* res == 0 means timeout, res < 0 is an error */
        if (res < 0) {
            FatalError("wait for input or timeout (select) failed");
        }

        /* res > 0: one or more descriptors are readable */
        else {
            for(i=0; i<FD_SETSIZE; i++) {

                /* check which descriptors are ready */
                if (FD_ISSET(i, &ReadFDs)) {

                    /* if it is the server socket, accept a new connection */
                    if (i == ServSocketFD) {

                        ClientLen = sizeof(ClientAddress);
                        DataSocketFD = accept(ServSocketFD, (struct sockaddr*)&ClientAddress, &ClientLen);
                
                        if (DataSocketFD < 0) {
                            FatalError("data socket creation (accept) failed");
                        }

                        FD_SET(DataSocketFD, &ActiveFDs);
                    } 

                    /* if it is a data socket, process a client request */
                    else {
                        disconnected = ProcessRequest(i, game, &ActiveFDs);

                        /* if the client disconnected or the player is not in the game, skip the rest of the loop */
                        if ((!disconnected) && findPlayerBySocket(game, i)) {
                            ProcessGameEvent(game, i);
                        }
                    }
                }
            }
        }
    }

    /* Close all connections if the server is shutting down. */
    for(i = 0; i < FD_SETSIZE; i++) {
        if(FD_ISSET(i, &ActiveFDs)) {
            close(i);
        }
    }
}


