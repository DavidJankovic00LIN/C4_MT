#pragma once

#include "board.h"
#include <cstdint>
#include <cstring>

class NegaMaxAI {
public:
    char board[BOARD_STORAGE_SIZE];
    int  PlayOut     = 0;
    int  EVA         = 0;
    bool provocation = false;

    NegaMaxAI() { std::memset(board, ' ', sizeof(board)); }

    int     GetValue(int column);
    uint8_t winning();
    int     AIManager();

private:
    int NegaMax(int Depth);
};
