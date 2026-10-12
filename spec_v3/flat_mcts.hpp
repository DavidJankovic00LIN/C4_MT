#pragma once

#include "board.hpp"

// Flat / pure Monte Carlo za figuru O.
// Vraća 1-based kolonu, ili 0 ako nema poteza.
int chooseColumnFlatMcts(char board[BOARD_STORAGE_SIZE], char me);
