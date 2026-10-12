#include "flat_mcts.hpp"

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

// Nasumično dovrši partiju; +1 ako pobedi me, -1 ako neko drugi, 0 nerešeno.
static int playout(char board[BOARD_STORAGE_SIZE], char me, char toMove) {
    while (true) {
        char w = findWinner(board);
        if (w != EMPTY)
            return (w == me) ? 1 : -1;
        if (isBoardFull(board))
            return 0;
        int col = randomLegalColumn(board);
        if (col == 0)
            return 0;
        applyMove(board, col, toMove);
        toMove = nextPiece(toMove);
    }
}

int chooseColumnFlatMcts(char board[BOARD_STORAGE_SIZE], char me) {
    int bestCol   = 0;
    double bestAvg = -1e300;

    for (int col = 1; col <= g_n; ++col) {
        if (!isLegalMove(board, col))
            continue;

        int sum = 0;
        for (int s = 0; s < SIMS_PER_MOVE; ++s) {
            char sim[BOARD_STORAGE_SIZE];
            std::memcpy(sim, board, BOARD_STORAGE_SIZE);
            applyMove(sim, col, me);
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
