#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "constants.h"
#include "server.h"

/****************************************************************************** Functions *******************************************************************************/

Card createCard(int rank, int suit) {

    /* local variable declaration */
    Card card;

    card.rank = rank;
    card.suit = suit;
    return card;
}

Deck createDeck() {

    /* local variable declaration */
    Deck deck;
    deck.topCardIndex = 0;
    int i = 0;

    for (int rank = RANK_2; rank <= RANK_ACE; rank++) {
        for (int suit = HEARTS; suit <= SPADES; suit++) {
            deck.cards[i] = createCard(rank, suit);
            i++;
        }
    }

    return deck;
}

/* This function should be updated by using "Fisher-Yates" algorithm */
void shuffleDeck(Deck* deck) {

    /* local variable declaration */
    Card temp;
    int j;

    for (int i = 0; i < NUM_CARDS; i++) {

        /* Generate a random number between 0 and 51 */
        j = rand() % NUM_CARDS;
        temp = deck->cards[i];
        deck->cards[i] = deck->cards[j];
        deck->cards[j] = temp;
    }

    /* Reset the topCardIndex */
    deck->topCardIndex = 0;
}

Card drawCard(Deck* deck) {
    if (deck->topCardIndex < NUM_CARDS) {

        /* Return the card at the topCardIndex and increment topCardIndex */
        return deck->cards[deck->topCardIndex++];
    } 
    else {
        
        /* Return a dummy card */
        return createCard(0, 0);
    }
}

int compareCards(const void *a, const void *b) {

    /* pointer type casting */
    Card *cardA = (Card *)a;
    Card *cardB = (Card *)b;

    /* positive if cardA > cardB */
    /* negative if cardA < cardB */
    /* zero if cardA == cardB */
    return (cardA->rank - cardB->rank);
}

int isRoyalFlush(Card *cards, int numCards) {
    for (int i = 0; i <= numCards - 5; i++) {
        if (cards[i].rank == RANK_10 &&
            cards[i+1].rank == RANK_JACK && cards[i+1].suit == cards[i].suit &&
            cards[i+2].rank == RANK_QUEEN && cards[i+2].suit == cards[i].suit &&
            cards[i+3].rank == RANK_KING && cards[i+3].suit == cards[i].suit &&
            cards[i+4].rank == RANK_ACE && cards[i+4].suit == cards[i].suit) {
            return 1;
        }
    }

    return 0;
}

int isStraightFlush(Card *cards, int numCards) {
    for (int i = 0; i <= numCards - 5; i++) {
        if (cards[i+4].rank - cards[i].rank == 4 &&
            cards[i+1].suit == cards[i].suit &&
            cards[i+2].suit == cards[i].suit &&
            cards[i+3].suit == cards[i].suit &&
            cards[i+4].suit == cards[i].suit) {
            return 1;
        }
    }

    /* Check for the special case baby straight "A2345" */
    if (cards[0].rank == RANK_2 && cards[1].rank == RANK_3 &&
        cards[2].rank == RANK_4 && cards[3].rank == RANK_5 &&
        cards[6].rank == RANK_ACE &&
        cards[0].suit == cards[1].suit && cards[1].suit == cards[2].suit &&
        cards[2].suit == cards[3].suit && cards[3].suit == cards[6].suit) {
        return 1;
    }

    return 0;
}

int isFourOfAKind(Card *cards, int numCards) {
    for (int i = 0; i <= numCards - 4; i++) {
        if (cards[i].rank == cards[i+1].rank &&
            cards[i+1].rank == cards[i+2].rank &&
            cards[i+2].rank == cards[i+3].rank) {
            return 1;
        }
    }
    return 0;
}

int isFullHouse(Card *cards, int numCards) {
    int threeKind = 0;
    int twoKind = 0;
    int rankOfThree = -1;
    
    for (int i = 0; i <= numCards - 3; i++) {
        if (cards[i].rank == cards[i+1].rank &&
            cards[i+1].rank == cards[i+2].rank) {
            threeKind = 1;
            rankOfThree = cards[i].rank;

            /* Skip the next two cards */
            i += 2; 
        }
    }

    for (int i = 0; i <= numCards - 2; i++) {
        if (cards[i].rank == cards[i+1].rank && cards[i].rank != rankOfThree) {
            twoKind = 1;

            /* Skip the next card */
            i++; 
        }
    }

    return (threeKind && twoKind);
}


int isFlush(Card *cards, int numCards) {

    /* Create an array to keep track of the suit counts */
    int suitCounts[SUITS_IN_DECK] = {0};

    /* Count the occurrences of each suit */
    for (int i = 0; i < numCards; i++) {
        suitCounts[cards[i].suit]++;
    }

    /* Check if any suit count is 5 or more */
    for (int i = 0; i < SUITS_IN_DECK; i++) {
        if (suitCounts[i] >= 5)
            return 1; 
    }

    return 0; 
}

int isStraight(Card *cards, int numCards) {
    for (int i = 0; i <= numCards - 5; i++) {
        if (cards[i+4].rank - cards[i].rank == 4) {
            return 1;
        }
    }

    /* Check for the special case baby straight "A2345" */
    if (cards[0].rank == RANK_2 && cards[1].rank == RANK_3 &&
        cards[2].rank == RANK_4 && cards[3].rank == RANK_5 &&
        cards[6].rank == RANK_ACE) {
        return 1;
    }

    return 0;
}

int isThreeOfAKind(Card *cards, int numCards) {
    for (int i = 0; i <= numCards - 3; i++) {
        if (cards[i].rank == cards[i+1].rank && cards[i].rank == cards[i+2].rank) {
            return 1;
        }
    }
    return 0;
}

int isTwoPair(Card *cards, int numCards) {
    int pairCount = 0;
    for (int i = 0; i <= numCards - 2; i++) {
        if (cards[i].rank == cards[i+1].rank) {
            pairCount++;

            /* Skip the next card */
            i++; 
        }
    }
    return pairCount >= 2;
}

int isPair(Card *cards, int numCards) {
    for (int i = 0; i <= numCards - 2; i++) {
        if (cards[i].rank == cards[i+1].rank) {
            return 1;
        }
    }
    return 0;
}

int findRankOfPair(Card *cards, int numCards) {
    for (int i = numCards - 1; i > 0; i--) {
        if (cards[i].rank == cards[i-1].rank) {
            return cards[i].rank;
        }
    }
    return -1;
}

int findRankOfHighPair(Card *cards, int numCards) {
    for (int i = numCards - 1; i > 0; i--) {
        if (cards[i].rank == cards[i-1].rank) {
            return cards[i].rank;
        }
    }
    return -1;
}

int findRankOfLowPair(Card *cards, int numCards) {
    int rankOfHighPair = findRankOfHighPair(cards, numCards);

    for (int i = numCards - 1; i > 0; i--) {
        if (cards[i].rank == cards[i-1].rank && cards[i].rank != rankOfHighPair) {
            return cards[i].rank;
        }
    }
    return -1;
}

int findRankOfThreeOfAKind(Card *cards, int numCards) {
    for (int i = numCards - 1; i >= 2; i--) {
        if (cards[i].rank == cards[i-1].rank && cards[i].rank == cards[i-2].rank) {
            return cards[i].rank;
        }
    }
    return -1;
}

int findHighCardInStraight(Card *cards, int numCards) {
    for (int i = numCards - 1; i > 3; i--) {
        if (cards[i].rank - cards[i-1].rank == 1 &&
            cards[i-1].rank - cards[i-2].rank == 1 &&
            cards[i-2].rank - cards[i-3].rank == 1 &&
            cards[i-3].rank - cards[i-4].rank == 1) {
            return cards[i].rank;
        }
    }

    /* Check for the special case baby straight "A2345" */
    if (cards[0].rank == RANK_2 && cards[1].rank == RANK_3 &&
        cards[2].rank == RANK_4 && cards[3].rank == RANK_5 &&
        cards[numCards-1].rank == RANK_ACE) {
        return RANK_5;
    }

    return -1;
}

int findHighCardInStraightFlush(Card *cards, int numCards) {
    return findHighCardInStraight(cards, numCards);
}

int findRankOfFourOfAKind(Card *cards, int numCards) {
    return cards[numCards - 1].rank;
}

int findRankOfTrioInFullHouse(Card *cards, int numCards) {
    return findRankOfThreeOfAKind(cards, numCards);
}

int findRankOfPairInFullHouse(Card *cards, int numCards) {
    int rankOfTrio = findRankOfTrioInFullHouse(cards, numCards);

    for (int i = numCards - 1; i > 0; i--) {
        if (cards[i].rank == cards[i+1].rank && cards[i].rank != rankOfTrio) {
            return cards[i].rank;
        }
    }
    return -1;
}

int findHighCardOutsideQuads(Card *cards, int numCards) {
    int quadRank = findRankOfFourOfAKind(cards, numCards);

    for (int i = numCards - 1; i >= 0; i--) {
        if (cards[i].rank != quadRank) {
            return cards[i].rank;
        }
    }
    return -1;
}

int findFlushSuit(Card *cards, int numCards) {
    int suitCounts[SUITS_IN_DECK] = {0}; 

    for (int i = 0; i < numCards; i++) {
        suitCounts[cards[i].suit]++;
    }

    for (int i = 0; i < 4; i++) {
        if (suitCounts[i] >= 5) {
            return i; 
        }
    }

    return -1; 
}

int findHighCard1InFlush(Card *cards, int numCards) {
    int flushSuit = findFlushSuit(cards, numCards);

    for (int i = numCards - 1; i >= 0; i--) {
        if (cards[i].suit == flushSuit) {
            return cards[i].rank;
        }
    }
    
    return -1;
}

int findHighCard2InFlush(Card *cards, int numCards) {
    int flushSuit = findFlushSuit(cards, numCards);
    int count = 0;

    for (int i = numCards - 1; i >= 0; i--) {
        if (cards[i].suit == flushSuit) {
            count++;
            if (count == 2) {
                return cards[i].rank;
            }
        }
    }

    return -1;
}

int findHighCard3InFlush(Card *cards, int numCards) {
    int flushSuit = findFlushSuit(cards, numCards);
    int count = 0;

    for (int i = numCards - 1; i >= 0; i--) {
        if (cards[i].suit == flushSuit) {
            count++;
            if (count == 3) {
                return cards[i].rank;
            }
        }
    }

    return -1;
}

int findHighCard4InFlush(Card *cards, int numCards) {
    int flushSuit = findFlushSuit(cards, numCards);
    int count = 0;

    for (int i = numCards - 1; i >= 0; i--) {
        if (cards[i].suit == flushSuit) {
            count++;
            if (count == 4) {
                return cards[i].rank;
            }
        }
    }

    return -1;
}

int findHighCard5InFlush(Card *cards, int numCards) {
    int flushSuit = findFlushSuit(cards, numCards);
    int count = 0;

    for (int i = numCards - 1; i >= 0; i--) {
        if (cards[i].suit == flushSuit) {
            count++;
            if (count == 5) {
                return cards[i].rank;
            }
        }
    }
    return -1;
}

int findHighCard1OutsideTrio(Card *cards, int numCards) {
    int trioRank = findRankOfThreeOfAKind(cards, numCards);

    for (int i = numCards - 1; i >= 0; i--) {
        if (cards[i].rank != trioRank) {
            return cards[i].rank;
        }
    }

    return -1;
}

int findHighCard2OutsideTrio(Card *cards, int numCards) {
    int trioRank = findRankOfThreeOfAKind(cards, numCards);
    int count = 0;

    for (int i = numCards - 1; i >= 0; i--) {
        if (cards[i].rank != trioRank) {
            count++;
            if (count == 2) {
                return cards[i].rank;
            }
        }
    }

    return -1;
}

int findHighCardOutsidePairs(Card *cards, int numCards) {
    int pairRank1 = findRankOfHighPair(cards, numCards);
    int pairRank2 = findRankOfLowPair(cards, numCards);

    /* Start from the end (highest cards) */
    for (int i = numCards - 1; i >= 0; i--) {
        if (cards[i].rank != pairRank1 && cards[i].rank != pairRank2) {
            return cards[i].rank;
        }
    }

    /* If no such card is found */
    return -1;
}

int findHighCard1OutsidePair(Card *cards, int numCards) {
    int pairRank = findRankOfPair(cards, numCards);

    /* Start from the end (highest cards) */
    for (int i = numCards - 1; i >= 0; i--) {
        if (cards[i].rank != pairRank) {
            return cards[i].rank;
        }
    }

    /* If no such card is found */
    return -1;
}

int findHighCard2OutsidePair(Card *cards, int numCards) {
    int pairRank = findRankOfPair(cards, numCards);
    int highCard1 = findHighCard1OutsidePair(cards, numCards);

    /* Start from the end (highest cards) */
    for (int i = numCards - 1; i >= 0; i--) {
        if (cards[i].rank != pairRank && cards[i].rank != highCard1) {
            return cards[i].rank;
        }
    }

    /* If no such card is found */
    return -1;
}

int findHighCard3OutsidePair(Card *cards, int numCards) {
    int pairRank = findRankOfPair(cards, numCards);
    int highCard1 = findHighCard1OutsidePair(cards, numCards);
    int highCard2 = findHighCard2OutsidePair(cards, numCards);

    /* Start from the end (highest cards) */
    for (int i = numCards - 1; i >= 0; i--) {
        if (cards[i].rank != pairRank && cards[i].rank != highCard1 && cards[i].rank != highCard2) {
            return cards[i].rank;
        }
    }

    /* If no such card is found */
    return -1;
}




