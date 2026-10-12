#include "paranoid_negamax.hpp"

#include <limits>

static constexpr int MAX_DEPTH = 4;

static int evaluateTerminal(char winner, char me) {
    if (winner == EMPTY)
        return 0;
    if (winner == me)
        return 1;
    return -1;
}

// Paranoid: maximize for me; any other player minimizes my score.
static int search(char board[BOARD_STORAGE_SIZE], char me, char toMove, int depth) {
    char winner = findWinner(board);
    if (winner != EMPTY)
        return evaluateTerminal(winner, me);
    if (depth > MAX_DEPTH || isBoardFull(board))
        return 0;

    const bool maximizing = (toMove == me);

    if (maximizing) {
        int best = std::numeric_limits<int>::min();
        bool any  = false;
        for (int col = 1; col <= g_n; ++col) {
            if (!isLegalMove(board, col))
                continue;
            int cell = lowestFreeCell(board, col);
            board[cell] = toMove;
            int score = search(board, me, nextPiece(toMove), depth + 1);
            board[cell] = EMPTY;
            if (!any || score > best)
                best = score;
            any = true;
            if (best == 1)
                break;
        }
        return any ? best : 0;
    }

    int best = std::numeric_limits<int>::max();
    bool any  = false;
    for (int col = 1; col <= g_n; ++col) {
        if (!isLegalMove(board, col))
            continue;
        int cell = lowestFreeCell(board, col);
        board[cell] = toMove;
        int score = search(board, me, nextPiece(toMove), depth + 1);
        board[cell] = EMPTY;
        if (!any || score < best)
            best = score;
        any = true;
        if (best == -1)
            break;
    }
    return any ? best : 0;
}

int chooseColumnParanoidNegamax(char board[BOARD_STORAGE_SIZE], char me) {
    int bestCol   = 0;
    int bestScore = std::numeric_limits<int>::min();

    for (int col = 1; col <= g_n; ++col) {
        if (!isLegalMove(board, col))
            continue;
        int cell = lowestFreeCell(board, col);
        board[cell] = me;
        int score = search(board, me, nextPiece(me), 1);
        board[cell] = EMPTY;
        if (bestCol == 0 || score > bestScore) {
            bestScore = score;
            bestCol   = col;
        }
        if (bestScore == 1)
            break;
    }
    return bestCol;
}
