# Andrew Nimegeers
# ann846
# 11294146
# CMPT 145
# Section 02

import random as r

class Card(object):
    def __init__(self):
        """
        Purpose:
            Initialize a Card object instance.
        """
        self.__deck = ['AS','2S','3S','4S','5S','6S','7S','8S','9S','10S','JS','QS','KS',
                       'AC','2C','3C','4C','5C','6C','7C','8C','9C','10C','JC','QC','KC',
                       'AH','2H','3H','4H','5H','6H','7H','8H','9H','10H','JH','QH','KH',
                       'AD','2D','3D','4D','5D','6D','7D','8D','9D','10D','JD','QD','KD']

    def create(self):
        """
        Purpose:
            Returns a list of all possible cards that can be drawn from the deck
        Post-conditions:
            (none)
        Return:
            A list of all possible cards that can be drawn from the deck
        """
        return self.__deck

    def deal(self, num_cards, num_players, deck):
        """
        Purpose:
            Randomly deal cards to a number of players
        Pre-conditions:
            num_cards: number of cards to be dealt
            num_players: number of players to deal to
            deck: deck of cards
        Post-conditions:
            (none)
        Return:
            a list of lists of each player's hand
        """
        if num_cards * num_players > 52:
            return 'There are not enough cards for all the players. Please lower the player count or card amount.'
        players_hands = []
        for i in range(num_players):
            hand = []
            for j in range(num_cards):
                card = r.choice(deck)
                hand.append(card)
                deck.remove(card)
            players_hands.append(hand)
        return players_hands

    def value(self, card):
        """
        Purpose:
            Returns the integer value of a card
        Pre-conditions:
            card: card to find integer value for
        Post-conditions:
            (none)
        Return:
            the integer value of a card
        """
        value = card[0]
        if value == 'A':
            value = 1
        elif value == '1':
            value = 10
        elif value == 'J':
            value = 11
        elif value == 'Q':
            value = 12
        elif value == 'K':
            value = 13
        else:
            value = int(value)
        return value

    def highest(self, list_of_cards):
        """
        Purpose:
            Returns the card that has the highest value
        Pre-conditions:
            list_of_cards: a list of cards
        Post-conditions:
            (none)
        Return:
            the card that has the highest value
        """
        duplicates = []
        highest_value = 0
        for i in list_of_cards:
            value = Card().value(i)
            if value > highest_value:
                highest_value = value
                highest_card = i
        for j in list_of_cards:
            if Card().value(j) == highest_value:
                duplicates.append(j)
        if len(duplicates) > 1:
            return duplicates
        else:
            return highest_card

    def lowest(self, list_of_cards):
        """
        Purpose:
            Returns the card that has the lowest value
        Pre-conditions:
            list_of_cards: a list of cards
        Post-conditions:
            (none)
        Return:
            the card that has the lowest value
        """
        duplicates = []
        lowest_value = 14
        for i in list_of_cards:
            value = Card().value(i)
            if value <= lowest_value:
                lowest_value = value
                lowest_card = i
        for j in list_of_cards:
            if Card().value(j) == lowest_value:
                duplicates.append(j)
        if len(duplicates) > 1:
            return duplicates
        else:
            return lowest_card

    def average(self, list_of_cards):
        """
        Purpose:
            Returns the average value of the given list of cards
        Pre-conditions:
            list_of_cards: a list of cards
        Post-conditions:
            (none)
        Return:
            the average value of the given list of cards
        """
        if len(list_of_cards) == 0:
            return 0
        total = 0
        for i in list_of_cards:
            total += Card().value(i)
        average = total / len(list_of_cards)
        return average
