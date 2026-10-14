#include "greedy.hpp"

#include <cstdlib>

static int countThreats(char board[BOARD_STORAGE_SIZE], char me) {
    int threats = 0;
    for (int col = 1; col <= g_n; ++col) {
        if (!isLegalMove(board, col))
            continue;
        int cell = lowestFreeCell(board, col);
        board[cell] = me;
        if (winsFromCell(board, cell))
            ++threats;
        board[cell] = EMPTY;
    }
    return threats;
}

int chooseColumnGreedy(char board[BOARD_STORAGE_SIZE], char me) {
    int win = findWinningColumn(board, me);
    if (win != 0)
        return win;
    int block = findBlockingColumn(board, me);
    if (block != 0)
        return block;

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
