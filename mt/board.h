#pragma once

#include <cstdint>

// Maksimalne dimenzije niza pri kompilaciji (mora biti >= kBoardSideMax).
#ifndef BOARD_ROWS_MAX
#define BOARD_ROWS_MAX 20
#endif
#ifndef BOARD_COLS_MAX
#define BOARD_COLS_MAX 20
#endif

static constexpr int kConnectLen     = 4;
static constexpr int kBoardSideMin     = 7;
static constexpr int kBoardSideMax   = 15;

static_assert(BOARD_ROWS_MAX >= kBoardSideMax, "BOARD_ROWS_MAX must be >= kBoardSideMax (15)");
static_assert(BOARD_COLS_MAX >= kBoardSideMax, "BOARD_COLS_MAX must be >= kBoardSideMax (15)");
static_assert(BOARD_ROWS_MAX >= kConnectLen, "BOARD_ROWS_MAX must be >= kConnectLen");
static_assert(BOARD_COLS_MAX >= kConnectLen, "BOARD_COLS_MAX must be >= kConnectLen");

#define BOARD_STORAGE_SIZE (BOARD_ROWS_MAX * BOARD_COLS_MAX + 1)

extern int g_board_rows;
extern int g_board_cols;

// Kvadratna tabla N x N; N u [kBoardSideMin, kBoardSideMax] i ne veće od BOARD_*_MAX.
bool boardSetSquareSize(int n);

inline int boardByteSize() { return g_board_rows * g_board_cols + 1; }

inline int cellIndex(int col1, int row0) { return col1 + g_board_cols * row0; }

inline void cellToColRow(int cell, int* col1, int* row0) {
    int c = cell % g_board_cols;
    if (c == 0)
        c = g_board_cols;
    *col1 = c;
    *row0 = (cell - c) / g_board_cols;
}

// Pobeda od poslednje odigrane ćelije. Nova četvorka mora da je sadrži.
inline bool winsFromCell(const char* board, int cell) {
    if (cell <= 0)
        return false;
    const char s = board[cell];
    if (s == ' ')
        return false;

    int col1 = 0;
    int row0 = 0;
    cellToColRow(cell, &col1, &row0);

    static const int kDirs[4][2] = {{1, 0}, {0, 1}, {1, 1}, {1, -1}};
    for (const auto& d : kDirs) {
        const int dc = d[0];
        const int dr = d[1];
        int count = 1;
        for (int k = 1; k < kConnectLen; ++k) {
            int c = col1 + dc * k;
            int r = row0 + dr * k;
            if (c < 1 || c > g_board_cols || r < 0 || r >= g_board_rows)
                break;
            if (board[cellIndex(c, r)] != s)
                break;
            ++count;
        }
        for (int k = 1; k < kConnectLen; ++k) {
            int c = col1 - dc * k;
            int r = row0 - dr * k;
            if (c < 1 || c > g_board_cols || r < 0 || r >= g_board_rows)
                break;
            if (board[cellIndex(c, r)] != s)
                break;
            ++count;
        }
        if (count >= kConnectLen)
            return true;
    }
    return false;
}

inline int lowestFreeCell(const char* board, int col1) {
    if (col1 < 1 || col1 > g_board_cols)
        return 0;
    for (int row = g_board_rows - 1; row >= 0; --row) {
        int idx = cellIndex(col1, row);
        if (board[idx] == ' ')
            return idx;
    }
    return 0;
}

inline bool isLegalMoveCol1(const char* board, int col1) {
    if (col1 < 1 || col1 > g_board_cols)
        return false;
    return board[cellIndex(col1, 0)] == ' ';
}

inline bool applyMoveCol1(char* board, int col1, char piece) {
    int cell = lowestFreeCell(board, col1);
    if (cell == 0)
        return false;
    board[cell] = piece;
    return true;
}

char boardFindWinner(const char board[BOARD_STORAGE_SIZE]);
bool boardIsFull(const char board[BOARD_STORAGE_SIZE]);
