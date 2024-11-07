#ifndef __GAMEBOARD_H__
#define __GAMEBOARD_H__

#define RESULT_HIT 1
#define RESULT_MISS 0
#define RESULT_REPEAT 2

typedef struct board_position {
    unsigned int row;       // Row coordinate; for a gameboard with
                            // size width x height, must not greater
                            // than (board height)-1.
    unsigned int col;       // Column coordinate; for a gameboard with
                            // size width x height, must not be greater
                            // than (board width)-1.
} BoardPosition;

typedef struct gameboard {
    unsigned int width;             // Board width; must be 5 <= width <= 26
    unsigned int height;            // Board width; must be 5 <= height <= 26
    unsigned int num_ships;         // Number of hidden ships; must be 5 <= hidden_ships <= 10
    unsigned int num_shots;         // Current number of shots taken by the player (initially zero)
    BoardPosition *grid_positions;  // Grid positions of the ships on the game board
    char **game_state;              // width x height grid of the game board; indicates if spaces have received a hit, miss, or not been shot at
} Gameboard;

Gameboard* create_gameboard(unsigned int width, unsigned int height); // Creates gameboard

void print_game_state(Gameboard* gameboard); // Prints gameboard

int check_for_hit(Gameboard* gameboard, BoardPosition position); // Checks if the shot was a hit, miss, or repeat

void record_hit(Gameboard* gameboard, BoardPosition position); // Updates the board for a hit

void record_miss(Gameboard* gameboard, BoardPosition position); // Updates the board for a miss

int is_victory(Gameboard* gameboard); // Checks if the player has sunk all the ships

#endif