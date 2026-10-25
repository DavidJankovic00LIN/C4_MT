#include "negamax.h"

int MAX_DEPTH = 6;

static constexpr int kAbBound = 4;

static void fillCenterOrder(int order[BOARD_COLS_MAX], int* count) {
    *count = 0;
    const int center = (g_board_cols + 1) / 2;
    for (int dist = 0; dist < g_board_cols; ++dist) {
        for (int col = 1; col <= g_board_cols; ++col) {
            int delta = col - center;
            if (delta < 0)
                delta = -delta;
            if (delta == dist)
                order[(*count)++] = col;
        }
    }
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
    float chance[2] = {9999999, 0};
    for (int column = 1; column <= g_board_cols; column++) {
        PlayOut = 0;
        EVA     = 0;
        int PlayNumber = GetValue(column);
        if (PlayNumber != 0) {
            board[PlayNumber] = 'O';
            if (winsFromCell(board, PlayNumber)) {
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
    return -search(Depth, -kAbBound, kAbBound);
}

int NegaMaxAI::search(int Depth, int alpha, int beta) {
    const char XO = (Depth % 2 != 0) ? 'X' : 'O';
    int order[BOARD_COLS_MAX];
    int orderCount = 0;
    fillCenterOrder(order, &orderCount);

    const bool recurse = Depth <= MAX_DEPTH;
    bool any = false;
    for (int i = 0; i < orderCount; ++i) {
        const int cell = GetValue(order[i]);
        if (cell == 0)
            continue;
        any = true;
        board[cell] = XO;
        if (winsFromCell(board, cell)) {
            PlayOut++;
            if (XO == 'O')
                EVA++;
            else
                EVA--;
            board[cell] = ' ';
            return 1;
        }
        if (!recurse) {
            board[cell] = ' ';
            continue;
        }
        const int v = -search(Depth + 1, -beta, -alpha);
        board[cell] = ' ';
        if (v > alpha)
            alpha = v;
        if (alpha >= beta)
            return alpha;
    }
    if (!recurse || !any)
        return 0;
    return alpha;
}
