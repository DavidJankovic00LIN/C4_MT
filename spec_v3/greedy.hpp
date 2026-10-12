#pragma once

#include "board.hpp"

// Greedy AI za figuru +.
// Vraća 1-based kolonu, ili 0 ako nema poteza.
int chooseColumnGreedy(char board[BOARD_STORAGE_SIZE], char me);
