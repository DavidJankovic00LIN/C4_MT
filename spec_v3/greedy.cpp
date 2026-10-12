#include "greedy.hpp"

#include <cstdlib>

static const char kOpponents[] = {PIECE_X, PIECE_O, PIECE_AT, PIECE_PLUS};

static int countThreats(char board[BOARD_STORAGE_SIZE], char me) {
    int threats = 0;
    for (int col = 1; col <= g_n; ++col) {
        if (!isLegalMove(board, col))
            continue;
        int cell = lowestFreeCell(board, col);
        board[cell] = me;
        if (findWinner(board) == me)
            ++threats;
        board[cell] = EMPTY;
    }
    return threats;
}

int chooseColumnGreedy(char board[BOARD_STORAGE_SIZE], char me) {
    // 1) Pobedi odmah
    for (int col = 1; col <= g_n; ++col) {
        if (!isLegalMove(board, col))
            continue;
        int cell = lowestFreeCell(board, col);
        board[cell] = me;
        bool win = (findWinner(board) == me);
        board[cell] = EMPTY;
        if (win)
            return col;
    }

    // 2) Blokiraj protivnika koji bi pobedio sledećim potezom
    for (char opp : kOpponents) {
        if (opp == me)
            continue;
        for (int col = 1; col <= g_n; ++col) {
            if (!isLegalMove(board, col))
                continue;
            int cell = lowestFreeCell(board, col);
            board[cell] = opp;
            bool oppWins = (findWinner(board) == opp);
            board[cell] = EMPTY;
            if (oppWins)
                return col;
        }
    }

    // 3) Preferiraj centar + malo pretnji
    int bestCol   = 0;
    int bestScore = -1000000;
    const int center = (g_n + 1) / 2;

    for (int col = 1; col <= g_n; ++col) {
        if (!isLegalMove(board, col))
            continue;
        int cell = lowestFreeCell(board, col);
        board[cell] = me;
        int score = 10 - std::abs(col - center);
        score += 2 * countThreats(board, me);
        board[cell] = EMPTY;
        if (bestCol == 0 || score > bestScore) {
            bestScore = score;
            bestCol   = col;
        }
    }
    return bestCol;
}
