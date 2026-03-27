#include "negamax.h"

int MAX_DEPTH = 6;

int NegaMaxAI::GetValue(int column) { return lowestFreeCell(board, column); }

uint8_t NegaMaxAI::winning() {
    char w = boardFindWinner(board);
    if (w == 'X')
        return 1;
    if (w == 'O')
        return 2;
    if (boardIsFull(board))
        return 3;
    return 0;
}

int NegaMaxAI::AIManager() {
    float chance[2] = {9999999, 0};
    for (int column = 1; column <= g_board_cols; column++) {
        PlayOut = 0;
        EVA     = 0;
        int PlayNumber = GetValue(column);
        if (PlayNumber != 0) {
            board[PlayNumber] = 'O';
            if (winning() == 2) {
                board[PlayNumber] = ' ';
                return PlayNumber;
            }
            float temp = -(100 * NegaMax(1));
            if (PlayOut != 0)
                temp -= ((100 * EVA) / PlayOut);
            if (-temp >= 100)
                provocation = true;
            if (chance[0] > temp) {
                chance[0] = temp;
                chance[1] = static_cast<float>(PlayNumber);
            }
            board[PlayNumber] = ' ';
        }
    }
    return static_cast<int>(chance[1]);
}

int NegaMaxAI::NegaMax(int Depth) {
    char XO;
    int  PlayNumber[BOARD_COLS_MAX + 1];
    for (int i = 0; i <= BOARD_COLS_MAX; ++i)
        PlayNumber[i] = 0;
    int chance = 0;

    if (Depth % 2 != 0)
        XO = 'X';
    else
        XO = 'O';

    for (int column = 1; column <= g_board_cols; column++)
        PlayNumber[column] = GetValue(column);

    for (int column = 1; column <= g_board_cols; column++) {
        if (PlayNumber[column] != 0) {
            board[PlayNumber[column]] = XO;
            if (winning() != 0) {
                PlayOut++;
                if (XO == 'O')
                    EVA++;
                else
                    EVA--;
                board[PlayNumber[column]] = ' ';
                return -1;
            }
            board[PlayNumber[column]] = ' ';
        }
    }

    if (Depth <= MAX_DEPTH) {
        for (int column = 1; column <= g_board_cols; column++) {
            int temp = 0;
            if (PlayNumber[column] != 0) {
                board[PlayNumber[column]] = XO;
                if (winning() != 0) {
                    PlayOut++;
                    if (XO == 'O')
                        EVA++;
                    else
                        EVA--;
                    board[PlayNumber[column]] = ' ';
                    return -1;
                }
                temp = NegaMax(Depth+1);
                if (column == 1)
                    chance = temp;
                if (chance < temp)
                    chance = temp;
                board[PlayNumber[column]] = ' ';
            }
        }
    }
    return -chance;
}
