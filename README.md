Texas Hold'em Poker Game
========================

Description
-----------
Welcome to the Version 1.0 of our Texas Hold'em Poker Software Game! This computer program allows you to play poker against other players over a network. It has a server-client architecture and uses the TCP/IP protocol for communication. The game follows the standard rules of Texas Hold'em poker. 

In this game, a minimum of 2 and a maximum of 6 players can play simultaneously. It also supports multiple bots. The game allows you to set various initial data, such as the number of players, the number of bots, initial chips, minimum bets, and more. If a player disconnects, the game replaces the disconnected player with a bot.

The current player will be marked as red

Authors
-------
- Yongye Li
- Mary Campbell
- Arjun Sivakumar
- Joseph Principe
- Cristian Pina Bravo
- Zijie Huang

Version
-------
This is the version 1.0 of our Poker software published on June 11, 2023.

Features
--------
* Network multiplayer play
* Room creation based on parameters
* Joining existing rooms
* Seat information retrieval
* Disconnected player is replaced by a bot

General Instructions
---------------------
1. Start the Server: Navigate to the bin directory and run the command: "./server #port", replacing #port with the port number you wish to use.

2. Start the Client: Navigate to the bin directory and run the command: "./poker servername #port", replacing "servername" with the name or IP address of the server, and #port with the port number used by the server.

3. Room Creation: Use the command "CREAT [room name] SEAT [seat number] PLAYER [player count] BOT [bot count] POINT [point count] ROUND [round count] SB [small blind]". For example, "CREAT Jacky SEAT 1 PLAYER 2 BOT 0 POINT 1000 ROUND 10 SB 5" represents creating a room with the following parameters:
   - [room name]: The name of player who creates the room.
   - [seat number]: The seat number assigned to the player who creates the room.
   - [player count]: The total number of players in the game, including bots and human players.
   - [bot count]: The number of bots in the game.
   - [point count]: The intial chip count for each player.
   - [round count]: The total number of rounds in the game.
   - [small blind]: The amount of the small blind bet.
4. Join Room: Use the command "JOIN [player name] SEAT [seat number]". For example, "JOIN Alice SEAT 2" indicates that the player named Alice joins the room at seat number 2.

5. Get Seat Info: Use the command "GET SEAT [seat number]". For example, "GET SEAT 3" fetches the information of seat number
 
6. Start Playing: Once connected and all players have joined, the game will start. Follow the text prompts to play the game. You can use the commands "BET [amount]", "CALL", "RAISE [amount]", "CHECK", "ALLIN", or "FOLD" to make your moves. These commands represent the following actions:
   - "BET [amount]": Place a bet of a certain amount.
   - "CALL": Match the current highest bet on the table.
   - "RAISE [amount]": Increase the current highest bet on the table by a certain amount.
   - "CHECK": Pass the action to the next player without making a bet. This is only allowed if no other player has made a bet in the current round.
   - "ALLIN": Bet all of one's remaining chips.
   - "FOLD": Surrender one's hand, forfeiting the current round.

Game Termination
-----------------
At the conclusion of the specified number of rounds, the server will automatically shut down. The clients will be presented with the game over screen. It's important to note that the server does not have to be manually terminated; it will shut down on its own once the game reaches the round limit set at the beginning of the game. Make sure to wrap up your game actions before the last round ends.

Note: For installation instructions, please refer to the INSTALL.txt.
