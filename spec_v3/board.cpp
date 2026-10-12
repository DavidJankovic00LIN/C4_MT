#include "board.hpp"

#include <cstring>

int g_n = 7;

bool boardSetSize(int n) {
    if (n < kBoardSideMin || n > kBoardSideMax)
        return false;
    if (n > BOARD_SIDE_MAX)
        return false;
    g_n = n;
    return true;
}

void clearBoard(char board[BOARD_STORAGE_SIZE]) {
    std::memset(board, EMPTY, BOARD_STORAGE_SIZE);
}

char findWinner(const char board[BOARD_STORAGE_SIZE]) {
    for (int row = 0; row < g_n; ++row) {
        for (int col1 = 1; col1 <= g_n - kConnectLen + 1; ++col1) {
            char s = board[cellIndex(col1, row)];
            if (s == EMPTY)
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
    for (int col1 = 1; col1 <= g_n; ++col1) {
        for (int row = 0; row <= g_n - kConnectLen; ++row) {
            char s = board[cellIndex(col1, row)];
            if (s == EMPTY)
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
    for (int row = 0; row <= g_n - kConnectLen; ++row) {
        for (int col1 = 1; col1 <= g_n - kConnectLen + 1; ++col1) {
            char s = board[cellIndex(col1, row)];
            if (s == EMPTY)
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
    for (int row = 0; row <= g_n - kConnectLen; ++row) {
        for (int col1 = kConnectLen; col1 <= g_n; ++col1) {
            char s = board[cellIndex(col1, row)];
            if (s == EMPTY)
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
    return EMPTY;
}

bool isBoardFull(const char board[BOARD_STORAGE_SIZE]) {
    for (int col1 = 1; col1 <= g_n; ++col1) {
        if (board[cellIndex(col1, 0)] == EMPTY)
            return false;
    }
    return true;
}
