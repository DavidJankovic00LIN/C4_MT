#include "mcts.hpp"

#include <cstdlib>
#include <cstring>

static constexpr int SIMS_PER_MOVE = 80;

static int randomLegalColumn(const char board[BOARD_STORAGE_SIZE]) {
    int legal[BOARD_SIDE_MAX];
    int count = 0;
    for (int c = 1; c <= g_n; ++c)
        if (isLegalMove(board, c))
            legal[count++] = c;
    if (count == 0)
        return 0;
    return legal[std::rand() % count];
}

static int playout(char board[BOARD_STORAGE_SIZE], char me, char toMove) {
    while (true) {
        if (isBoardFull(board))
            return 0;
        int col = randomLegalColumn(board);
        if (col == 0)
            return 0;
        int cell = applyMoveCell(board, col, toMove);
        if (cell == 0)
            return 0;
        if (winsFromCell(board, cell))
            return (toMove == me) ? 1 : -1;
        toMove = nextPiece(toMove);
    }
}

int chooseColumnMcts(char board[BOARD_STORAGE_SIZE], char me) {
    int win = findWinningColumn(board, me);
    if (win != 0)
        return win;
    int block = findBlockingColumn(board, me);
    if (block != 0)
        return block;

    int bestCol    = 0;
    double bestAvg = -1e300;

    for (int col = 1; col <= g_n; ++col) {
        if (!isLegalMove(board, col))
            continue;

        int sum = 0;
        for (int s = 0; s < SIMS_PER_MOVE; ++s) {
            char sim[BOARD_STORAGE_SIZE];
            std::memcpy(sim, board, BOARD_STORAGE_SIZE);
            int cell = applyMoveCell(sim, col, me);
            if (cell == 0)
                continue;
            if (winsFromCell(sim, cell)) {
                sum += 1;
                continue;
            }
            sum += playout(sim, me, nextPiece(me));
        }
        double avg = static_cast<double>(sum) / SIMS_PER_MOVE;
        if (bestCol == 0 || avg > bestAvg) {
            bestAvg = avg;
            bestCol = col;
        }
    }
    return bestCol;
}
