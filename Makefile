#######################################################################
# Poker Program, for team10 of EECS 22L 
#
# Makefile: Makefile for Poker Program
#
#######################################################################

# Variables
CC	= gcc
CFLAGS	= -Wall -std=c11 -g
GTKFLAGS = `pkg-config --cflags --libs gtk+-2.0` -lm

# Default target
all: bin/server bin/poker

######################### Generate object files #######################

# Target for card.o
src/card.o: src/card.c src/constants.h src/server.h
	$(CC) $(CFLAGS) -c src/card.c -o src/card.o

# Target for game.o
src/game.o: src/game.c src/constants.h src/server.h
	$(CC) $(CFLAGS) -c src/game.c -o src/game.o

# Target for player.o
src/player.o: src/player.c src/constants.h src/server.h
	$(CC) $(CFLAGS) -c src/player.c -o src/player.o

# Target for gui.o
src/gui.o: src/gui.c src/client.h
	$(CC) $(CFLAGS) $(GTKFLAGS) -c src/gui.c -o src/gui.o

# Target for server.o
src/server.o: src/server.c src/constants.h src/server.h
	$(CC) $(CFLAGS) -c src/server.c -o src/server.o

# Target for client.o
src/client.o: src/client.c src/constants.h src/client.h
	$(CC) $(CFLAGS) -c src/client.c -o src/client.o

######################### Generate the executable #####################

# Create bin directory
bin:
	mkdir -p $@

# Target for server
bin/server: bin src/card.o src/game.o src/player.o src/server.o
	$(CC) $(CFLAGS) src/card.o src/game.o src/player.o src/server.o -o bin/server

# Target for poker
bin/poker: bin src/client.o src/gui.o
	$(CC) $(CFLAGS) $(GTKFLAGS) src/client.o src/gui.o -o bin/poker

###############################  others  ##############################

# Target for run
run-server: 
	bin/server
	./bin/server

run-client:
	bin/poker
	./bin/poker

# Target for clean-up
clean:
	rm -f src/*.o bin/server bin/poker

# Target for test
test-gui:
	@echo "Test Steps for GUI:"
	@echo "Just connect the server and the client, and the GUI will show up."

test-comm:
	@echo "Test Steps for Server-Client Communication:"
	@echo "1. Start the server: Navigate to the bin directory and run the command: './server #port'."
	@echo "2. Start the client: Once the server is running, clients can connect to it. Navigate to the bin directory and run the command: './poker servername #port'."
	@echo
	@echo "For example:"
	@echo "./server 10000"
	@echo "./poker bondi 10000"

# Target for tar
tar:
	tar czvf src/poker_source_code.tar.gz src/*.c src/*.h
	echo "Poker source code tarball created in the src directory."

