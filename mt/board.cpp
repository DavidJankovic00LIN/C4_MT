#include "board.h"

int g_board_rows = 7;
int g_board_cols = 7;

bool boardSetSquareSize(int n) {
    if (n < kBoardSideMin || n > kBoardSideMax)
        return false;
    if (n > BOARD_ROWS_MAX || n > BOARD_COLS_MAX)
        return false;
    g_board_rows = n;
    g_board_cols = n;
    return true;
}

char boardFindWinner(const char board[BOARD_STORAGE_SIZE]) {
    for (int row = 0; row < g_board_rows; ++row) {
        for (int col1 = 1; col1 <= g_board_cols - kConnectLen + 1; ++col1) {
            char s = board[cellIndex(col1, row)];
            if (s == ' ')
                continue;
            bool ok = true;
            for (int k = 1; k < kConnectLen; ++k) {
                if (board[cellIndex(col1 + k, row)] != s) {
                    ok = false;
                    break;
                }
            }
            if (ok)
                return s;
        }
    }
    for (int col1 = 1; col1 <= g_board_cols; ++col1) {
        for (int row = 0; row <= g_board_rows - kConnectLen; ++row) {
            char s = board[cellIndex(col1, row)];
            if (s == ' ')
                continue;
            bool ok = true;
            for (int k = 1; k < kConnectLen; ++k) {
                if (board[cellIndex(col1, row + k)] != s) {
                    ok = false;
                    break;
                }
            }
            if (ok)
                return s;
        }
    }
    for (int row = 0; row <= g_board_rows - kConnectLen; ++row) {
        for (int col1 = 1; col1 <= g_board_cols - kConnectLen + 1; ++col1) {
            char s = board[cellIndex(col1, row)];
            if (s == ' ')
                continue;
            bool ok = true;
            for (int k = 1; k < kConnectLen; ++k) {
                if (board[cellIndex(col1 + k, row + k)] != s) {
                    ok = false;
                    break;
                }
            }
            if (ok)
                return s;
        }
    }
    for (int row = 0; row <= g_board_rows - kConnectLen; ++row) {
        for (int col1 = kConnectLen; col1 <= g_board_cols; ++col1) {
            char s = board[cellIndex(col1, row)];
            if (s == ' ')
                continue;
            bool ok = true;
            for (int k = 1; k < kConnectLen; ++k) {
                if (board[cellIndex(col1 - k, row + k)] != s) {
                    ok = false;
                    break;
                }
            }
            if (ok)
                return s;
        }
    }
    return ' ';
}

bool boardIsFull(const char board[BOARD_STORAGE_SIZE]) {
    for (int col1 = 1; col1 <= g_board_cols; ++col1) {
        if (board[cellIndex(col1, 0)] == ' ')
            return false;
    }
    return true;
}
