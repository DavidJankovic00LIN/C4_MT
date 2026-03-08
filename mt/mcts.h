#pragma once

#include <vector>
#include <cmath>
#include <cstdint>
#include <limits>

// Board dimensions and piece symbols
static constexpr int ROWS    = 6;
static constexpr int COLS    = 7;
static constexpr int CELLS   = ROWS * COLS;  // 42

static constexpr char EMPTY  = ' ';
static constexpr char AI     = 'O';   // AI player  (GREEN in Java)
static constexpr char HUMAN  = 'X';   // Human player (BLUE in Java)

// ─────────────────────────────────────────────
//  Board helpers (operate on a 42-char array)
// ─────────────────────────────────────────────

// Returns the lowest free row index for a given column (1-indexed col),
// or 0 if the column is full / out of range.
int  getLowestFreeCell(const char board[CELLS], int col);

// Returns true if at least one cell in the column is free.
bool isLegalMove(const char board[CELLS], int col);

// Places piece on the board. Returns false if the move was illegal.
bool applyMove(char board[CELLS], int col, char piece);

// Returns the winner piece ('X', 'O') or EMPTY if no winner yet.
char findWinner(const char board[CELLS]);

// Returns true if every cell is occupied (draw).
bool isBoardFull(const char board[CELLS]);


// ─────────────────────────────────────────────
//  MCTS Node
// ─────────────────────────────────────────────
struct Node {
    int  col;           // Column this node represents (-1 for root)
    int  wins;          // Accumulated win score
    int  moves;         // Number of moves to terminal state across rollouts
    int  visits;        // How many times this node has been visited
    double ucbValue;    // Cached UCB1 value (updated in back-propagation)

    Node*              parent;
    std::vector<Node*> childArray;

    explicit Node(int col = -1, Node* parent = nullptr);
    ~Node();

    // Non-copyable to avoid accidental double-free of children
    Node(const Node&)            = delete;
    Node& operator=(const Node&) = delete;
};


// ─────────────────────────────────────────────
//  Monte Carlo Tree Search
// ─────────────────────────────────────────────
class MonteCarloTreeSearch {
public:
    MonteCarloTreeSearch(const char currentBoard[CELLS], int iterations = 500);

    // Run MCTS and return the best column (0-indexed).
    int bestMove();

    // Set how many MCTS iterations to run per call to bestMove().
    void setIterations(int n) { iterations_ = n; }

    bool finalMove;   // true if AI found a forced win in one move

private:
    char  board_[CELLS];   // snapshot of the board at decision time
    int   iterations_;

    Node*              rootNode_;
    std::vector<Node*> maxUcbNodes_;

    // ── MCTS phases ──────────────────────────
    void   select       (Node* parentNode);
    void   expand       (Node* parentNode);
    void   rollOut      (Node* nonVisitedNode);
    void   backPropagate(Node* rolledOutNode, int win, int moves);
    void   updateMaxUcbNode(Node* node);

    // Returns a random legal column on board, or -1 if board is full.
    int    randomLegalMove(const char board[CELLS]) const;
};
