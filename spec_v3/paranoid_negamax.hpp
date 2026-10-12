#pragma once

#include "board.hpp"

// Paranoid NegaMax za figuru X (ili bilo koju — me = PIECE_X u igri).
// Vraća 1-based kolonu, ili 0 ako nema poteza.
int chooseColumnParanoidNegamax(char board[BOARD_STORAGE_SIZE], char me);
