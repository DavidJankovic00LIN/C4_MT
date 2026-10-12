#pragma once

#include "board.hpp"

// NegaMax za figuru X.
// Vraća 1-based kolonu, ili 0 ako nema poteza.
int chooseColumnNegamax(char board[BOARD_STORAGE_SIZE], char me);
