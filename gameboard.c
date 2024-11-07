
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <err.h>
#include "gameboard.h"
#include "rand_num.h"

/**
 * Creates the gameboard structure
 * 
 * @return a gameboard structure
 */
Gameboard* create_gameboard(unsigned int width, unsigned int height) {
	Gameboard* gameboard = (Gameboard*) malloc(sizeof(Gameboard)); // Allocate gameboard structure
	gameboard->width = width;
	gameboard->height = height;
	do {
	gameboard->num_ships = rand_num(10); // Randomly determine number of hidden ships
	} while (gameboard->num_ships < 5);
	gameboard->num_shots = 0; // Current number of shots is initially 0
	gameboard->grid_positions = (BoardPosition*) malloc(sizeof(BoardPosition) * gameboard->num_ships); // Allocate BoardPosition structure
	for (size_t i = 0; i < gameboard->num_ships; i++) {
		gameboard->grid_positions[i].row = rand_num(gameboard->height - 1); // Randomly assign row number to each hidden ship
		gameboard->grid_positions[i].col = rand_num(gameboard->width - 1); // Randomly assign column number to each hidden ship
		for (size_t j = 0; j < i; j++) {
			while (gameboard->grid_positions[i].row == gameboard->grid_positions[j].row && gameboard->grid_positions[i].col == gameboard->grid_positions[j].col) {
				gameboard->grid_positions[i].row = rand_num(gameboard->height - 1); // Randomly assign row number to each hidden ship
				gameboard->grid_positions[i].col = rand_num(gameboard->width - 1); // Randomly assign column number to each hidden ship
			}
		}
	}
	// Allocate game_state array
	gameboard->game_state = (char**) malloc(sizeof(char*) * gameboard->height);
	for (size_t k = 0; k < gameboard->height; k++) {
		gameboard->game_state[k] = (char*) malloc(sizeof(char) * gameboard->width);
	}
	// Initialize game_state to be all '.'
	for (size_t l = 0; l < gameboard->height; l++) {
		for (size_t m = 0; m < gameboard->width; m++) {
			gameboard->game_state[l][m] = '.';
		}
	}
	return gameboard;
}

/**
 * Prints the gameboard
 */ 
void print_game_state(Gameboard* gameboard) {
	for (size_t i = 0; i < gameboard->height; i++) {
		for (size_t j = 0; j < gameboard->width; j++) {
			printf("%c", gameboard->game_state[i][j]);
		}
		printf("\n");
	}

}

/**
 * Checks if the shot was a hit, repeat, or miss
 * 
 * @return A macro corresponding to hit, repeat, or miss
 */
int check_for_hit(Gameboard* gameboard, BoardPosition position) {
	if (position.row <= (gameboard->height - 1) && position.col <= (gameboard->width - 1)) {
		gameboard->num_shots += 1; // Update number of shots only if a valid shot is taken
	}
	for (size_t i = 0; i < gameboard->num_ships; i++) {
		if ((position.row == gameboard->grid_positions[i].row) && (position.col == gameboard->grid_positions[i].col) && (gameboard->game_state[position.row][position.col] == '.')) {
			return RESULT_HIT;
		}
	}
	if (gameboard->game_state[position.row][position.col] == 'x' || gameboard->game_state[position.row][position.col] == 'o') {
			return RESULT_REPEAT;
	}
	else {
			return RESULT_MISS;
	}
}

/**
 * Update position on gameboard to reflect a hit
 */
void record_hit(Gameboard* gameboard, BoardPosition position) {
	gameboard->game_state[position.row][position.col] = 'x';
}

/**
 * Update position on gameboard to reflect a miss
 */
void record_miss(Gameboard* gameboard, BoardPosition position) {
	gameboard->game_state[position.row][position.col] = 'o';
}

/**
 * Check if the user has sunk all the ships
 * 
 * @return 1 if they sunk all ships, 0 if not
 */
int is_victory(Gameboard* gameboard) {
	unsigned int current_ships_sunk = 0;
	for (size_t i = 0; i < gameboard->num_ships; i++) {
		if (gameboard->game_state[gameboard->grid_positions[i].row][gameboard->grid_positions[i].col] == 'x') {
			current_ships_sunk += 1; // count how many ships have been sunk
		}
	}
	if (current_ships_sunk == gameboard->num_ships) {
		return 1;
	}
	else {
		return 0;
	}
}
