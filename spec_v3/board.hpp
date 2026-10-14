#pragma once

#ifndef BOARD_SIDE_MAX
#define BOARD_SIDE_MAX 20
#endif

static constexpr int kConnectLen   = 4;
static constexpr int kBoardSideMin = 7;
static constexpr int kBoardSideMax = 15;

static_assert(BOARD_SIDE_MAX >= kBoardSideMax, "BOARD_SIDE_MAX must be >= 15");

#define BOARD_STORAGE_SIZE (BOARD_SIDE_MAX * BOARD_SIDE_MAX + 1)

static constexpr char EMPTY      = ' ';
static constexpr char PIECE_X    = 'X';
static constexpr char PIECE_O    = 'O';
static constexpr char PIECE_PLUS = '+';
static constexpr char PIECE_AT   = '@';

// Aktivna veličina N (kvadrat N x N).
extern int g_n;

bool boardSetSize(int n);

inline int boardByteSize() { return g_n * g_n + 1; }

// NegaMax layout: index 0 unused; col1 in [1,g_n], row0 = top
inline int cellIndex(int col1, int row0) { return col1 + g_n * row0; }

inline int lowestFreeCell(const char* board, int col1) {
    if (col1 < 1 || col1 > g_n)
        return 0;
    for (int row = g_n - 1; row >= 0; --row) {
        int idx = cellIndex(col1, row);
        if (board[idx] == EMPTY)
            return idx;
    }
    return 0;
}

inline bool isLegalMove(const char* board, int col1) {
    if (col1 < 1 || col1 > g_n)
        return false;
    return board[cellIndex(col1, 0)] == EMPTY;
}

inline bool applyMove(char* board, int col1, char piece) {
    int cell = lowestFreeCell(board, col1);
    if (cell == 0)
        return false;
    board[cell] = piece;
    return true;
}

// Kao applyMove, ali vraća index ćelije (0 ako nelegalno).
inline int applyMoveCell(char* board, int col1, char piece) {
    int cell = lowestFreeCell(board, col1);
    if (cell == 0)
        return 0;
    board[cell] = piece;
    return cell;
}

inline void cellToColRow(int cell, int* col1, int* row0) {
    int c = cell % g_n;
    if (c == 0)
        c = g_n;
    *col1 = c;
    *row0 = (cell - c) / g_n;
}

// Pobeda od poslednje odigrane ćelije (O(kConnectLen) po pravcu).
inline bool winsFromCell(const char* board, int cell) {
    if (cell <= 0)
        return false;
    const char s = board[cell];
    if (s == EMPTY)
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
            if (c < 1 || c > g_n || r < 0 || r >= g_n)
                break;
            if (board[cellIndex(c, r)] != s)
                break;
            ++count;
        }
        for (int k = 1; k < kConnectLen; ++k) {
            int c = col1 - dc * k;
            int r = row0 - dr * k;
            if (c < 1 || c > g_n || r < 0 || r >= g_n)
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

void clearBoard(char board[BOARD_STORAGE_SIZE]);

// Vraća figuricu pobednika ili EMPTY ako nema (full-board scan; za retke pozive).
char findWinner(const char board[BOARD_STORAGE_SIZE]);

bool isBoardFull(const char board[BOARD_STORAGE_SIZE]);

// Redosled igrača: X, O, +, @
inline char nextPiece(char piece) {
    if (piece == PIECE_X)
        return PIECE_O;
    if (piece == PIECE_O)
        return PIECE_PLUS;
    if (piece == PIECE_PLUS)
        return PIECE_AT;
    return PIECE_X;
}

// Kolona kojom me odmah pobedi; 0 ako nema.
inline int findWinningColumn(char* board, char me) {
    for (int col = 1; col <= g_n; ++col) {
        if (!isLegalMove(board, col))
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

// Kolona kojom neki protivnik odmah pobedi (za blok); 0 ako nema.
inline int findBlockingColumn(char* board, char me) {
    static const char kAll[] = {PIECE_X, PIECE_O, PIECE_PLUS, PIECE_AT};
    for (char opp : kAll) {
        if (opp == me)
            continue;
        for (int col = 1; col <= g_n; ++col) {
            if (!isLegalMove(board, col))
                continue;
            int cell = lowestFreeCell(board, col);
            board[cell] = opp;
            bool oppWins = winsFromCell(board, cell);
            board[cell] = EMPTY;
            if (oppWins)
                return col;
        }
    }
    return 0;
}
