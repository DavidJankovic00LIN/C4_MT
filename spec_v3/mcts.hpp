#pragma once

#include "board.hpp"

// MCTS za figuru O.
// Vraća 1-based kolonu, ili 0 ako nema poteza.
int chooseColumnMcts(char board[BOARD_STORAGE_SIZE], char me);
