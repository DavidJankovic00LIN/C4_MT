#include "negamax.h"

#include <cmath>
#include <limits>

static constexpr int MAX_DEPTH = 6;
static constexpr char EMPTY = ' ';
static constexpr char AI_PIECE = 'O';
static constexpr char HUMAN_PIECE = 'X';

static constexpr int kScoreWin  = 10000;
static constexpr int kScoreLoss = -10000;
static constexpr int kThreatWeight = 100;
static constexpr int kAbBound = kScoreWin + 1;

static char nextToMove(char piece) {
    return (piece == AI_PIECE) ? HUMAN_PIECE : AI_PIECE;
}

static void fillCenterOrder(int order[BOARD_COLS_MAX], int* count) {
    *count = 0;
    const int center = (g_board_cols + 1) / 2;
    for (int dist = 0; dist < g_board_cols; ++dist) {
        for (int col = 1; col <= g_board_cols; ++col) {
            if (std::abs(col - center) == dist)
                order[(*count)++] = col;
        }
    }
}

static int findWinningColumn(char* board, char me) {
    for (int col = 1; col <= g_board_cols; ++col) {
        if (!isLegalMoveCol1(board, col))
            continue;
        int cell = lowestFreeCell(board, col);
        board[cell] = me;
        bool win = winsFromCell(board, cell);
        board[cell] = EMPTY;
        if (win)
            return col;
    }
    return 0;
}

static int findBlockingColumn(char* board, char me) {
    const char opp = nextToMove(me);
    for (int col = 1; col <= g_board_cols; ++col) {
        if (!isLegalMoveCol1(board, col))
            continue;
        int cell = lowestFreeCell(board, col);
        board[cell] = opp;
        bool oppWins = winsFromCell(board, cell);
        board[cell] = EMPTY;
        if (oppWins)
            return col;
    }
    return 0;
}

static int countThreats(char* board, char me) {
    int threats = 0;
    for (int col = 1; col <= g_board_cols; ++col) {
        if (!isLegalMoveCol1(board, col))
            continue;
        int cell = lowestFreeCell(board, col);
        board[cell] = me;
        if (winsFromCell(board, cell))
            ++threats;
        board[cell] = EMPTY;
    }
    return threats;
}

static int evaluateBoard(char* board, char me) {
    const char opp = nextToMove(me);
    int score = kThreatWeight * countThreats(board, me)
                - kThreatWeight * countThreats(board, opp);

    const int center = (g_board_cols + 1) / 2;
    for (int col = 1; col <= g_board_cols; ++col) {
        for (int row = 0; row < g_board_rows; ++row) {
            int idx = cellIndex(col, row);
            if (board[idx] != me)
                continue;
            int dist = col - center;
            if (dist < 0)
                dist = -dist;
            int bonus = 3 - dist;
            if (bonus > 0)
                score += bonus;
        }
    }
    return score;
}

static int evaluateTerminal(char winner, char me) {
    if (winner == EMPTY)
        return 0;
    if (winner == me)
        return kScoreWin;
    return kScoreLoss;
}

static int search(char board[BOARD_STORAGE_SIZE], char me, char toMove, int depth,
                  const int* order, int orderCount, int lastCell, int alpha, int beta) {
    if (lastCell != 0 && winsFromCell(board, lastCell))
        return evaluateTerminal(board[lastCell], me);
    if (depth > MAX_DEPTH || boardIsFull(board))
        return evaluateBoard(board, me);

    const bool maximizing = (toMove == me);

    if (maximizing) {
        int best = std::numeric_limits<int>::min();
        bool any  = false;
        for (int i = 0; i < orderCount; ++i) {
            int col = order[i];
            if (!isLegalMoveCol1(board, col))
                continue;
            int cell = lowestFreeCell(board, col);
            board[cell] = toMove;
            int score = search(board, me, nextToMove(toMove), depth + 1, order, orderCount, cell,
                               alpha, beta);
            board[cell] = EMPTY;
            if (!any || score > best)
                best = score;
            any = true;
            if (score > alpha)
                alpha = score;
            if (alpha >= beta || best == kScoreWin)
                break;
        }
        return any ? best : evaluateBoard(board, me);
    }

    int best = std::numeric_limits<int>::max();
    bool any  = false;
    for (int i = 0; i < orderCount; ++i) {
        int col = order[i];
        if (!isLegalMoveCol1(board, col))
            continue;
        int cell = lowestFreeCell(board, col);
        board[cell] = toMove;
        int score = search(board, me, nextToMove(toMove), depth + 1, order, orderCount, cell,
                           alpha, beta);
        board[cell] = EMPTY;
        if (!any || score < best)
            best = score;
        any = true;
        if (score < beta)
            beta = score;
        if (alpha >= beta || best == kScoreLoss)
            break;
    }
    return any ? best : evaluateBoard(board, me);
}

static int chooseColumn(char board[BOARD_STORAGE_SIZE], char me) {
    int win = findWinningColumn(board, me);
    if (win != 0)
        return win;
    int block = findBlockingColumn(board, me);
    if (block != 0)
        return block;

    int order[BOARD_COLS_MAX];
    int orderCount = 0;
    fillCenterOrder(order, &orderCount);

    int bestCol   = 0;
    int bestScore = std::numeric_limits<int>::min();

    for (int i = 0; i < orderCount; ++i) {
        int col = order[i];
        if (!isLegalMoveCol1(board, col))
            continue;
        int cell = lowestFreeCell(board, col);
        board[cell] = me;
        int score = search(board, me, nextToMove(me), 1, order, orderCount, cell,
                           -kAbBound, kAbBound);
        board[cell] = EMPTY;
        if (bestCol == 0 || score > bestScore) {
            bestScore = score;
            bestCol   = col;
        }
        if (bestScore == kScoreWin)
            break;
    }
    return bestCol;
}

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
    int col = chooseColumn(board, AI_PIECE);
    if (col == 0)
        return 0;
    return lowestFreeCell(board, col);
}
