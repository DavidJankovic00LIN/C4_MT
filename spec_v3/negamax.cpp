#include "negamax.hpp"

#include <cmath>
#include <limits>

static constexpr int MAX_DEPTH = 4;

static void fillCenterOrder(int order[BOARD_SIDE_MAX], int* count) {
    *count = 0;
    const int center = (g_n + 1) / 2;
    for (int dist = 0; dist < g_n; ++dist) {
        for (int col = 1; col <= g_n; ++col) {
            if (std::abs(col - center) == dist)
                order[(*count)++] = col;
        }
    }
}

static int evaluateTerminal(char winner, char me) {
    if (winner == EMPTY)
        return 0;
    if (winner == me)
        return 1;
    return -1;
}

static int search(char board[BOARD_STORAGE_SIZE], char me, char toMove, int depth,
                  const int* order, int orderCount) {
    char winner = findWinner(board);
    if (winner != EMPTY)
        return evaluateTerminal(winner, me);
    if (depth > MAX_DEPTH || isBoardFull(board))
        return 0;

    const bool maximizing = (toMove == me);

    if (maximizing) {
        int best = std::numeric_limits<int>::min();
        bool any  = false;
        for (int i = 0; i < orderCount; ++i) {
            int col = order[i];
            if (!isLegalMove(board, col))
                continue;
            int cell = lowestFreeCell(board, col);
            board[cell] = toMove;
            int score = search(board, me, nextPiece(toMove), depth + 1, order, orderCount);
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
    for (int i = 0; i < orderCount; ++i) {
        int col = order[i];
        if (!isLegalMove(board, col))
            continue;
        int cell = lowestFreeCell(board, col);
        board[cell] = toMove;
        int score = search(board, me, nextPiece(toMove), depth + 1, order, orderCount);
        board[cell] = EMPTY;
        if (!any || score < best)
            best = score;
        any = true;
        if (best == -1)
            break;
    }
    return any ? best : 0;
}

int chooseColumnNegamax(char board[BOARD_STORAGE_SIZE], char me) {
    int win = findWinningColumn(board, me);
    if (win != 0)
        return win;
    int block = findBlockingColumn(board, me);
    if (block != 0)
        return block;

    int order[BOARD_SIDE_MAX];
    int orderCount = 0;
    fillCenterOrder(order, &orderCount);

    int bestCol   = 0;
    int bestScore = std::numeric_limits<int>::min();

    for (int i = 0; i < orderCount; ++i) {
        int col = order[i];
        if (!isLegalMove(board, col))
            continue;
        int cell = lowestFreeCell(board, col);
        board[cell] = me;
        int score = search(board, me, nextPiece(me), 1, order, orderCount);
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
