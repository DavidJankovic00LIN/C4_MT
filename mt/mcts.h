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
//  MCTS Node (src-style: score for UCT and best move)
// ─────────────────────────────────────────────
struct Node {
    int    col;     // Column this node represents (-1 for root)
    double score;  // Accumulated simulation result (AI win +1, draw 0, opponent -2)
    int    visits;  // Number of simulations through this node
    Node*  parent;
    std::vector<Node*> childArray;

    explicit Node(int col = -1, Node* parent = nullptr);
    ~Node();

    Node(const Node&)            = delete;
    Node& operator=(const Node&) = delete;
};


// ─────────────────────────────────────────────
//  Monte Carlo Tree Search (algorithm from src)
// ─────────────────────────────────────────────
class MonteCarloTreeSearch {
public:
    MonteCarloTreeSearch(const char currentBoard[CELLS], int iterations = 50000);
    ~MonteCarloTreeSearch();

    // Run MCTS and return the best column (0-indexed).
    int bestMove();

    void setIterations(int n) { iterations_ = n; }

    bool finalMove;   // true if AI found a forced win in one move

private:
    char board_[CELLS];
    int  iterations_;
    static constexpr int SIMS_PER_EXPAND = 100;  // src: 100 simulations per expanded node

    Node* rootNode_;

    // Replay from root to node to get board state at node; returns whose turn is next (true = AI).
    void getBoardAtNode(Node* node, char out[CELLS], bool* turnIsAI) const;
    // UCT as in src: score/visits + 1.41*sqrt(log(parent->visits)/visits), unvisited = +inf
    double getUCT(Node* child) const;
    Node*  getBestChild(Node* parent) const;

    void runIteration();
    void expand(Node* leaf);
    // Simulate from board with given turn; return 1 (AI win), 0 (draw), -2 (opponent win).
    double simulate(char board[CELLS], bool turnIsAI) const;
    void   backPropagate(Node* node, double value);

    int randomLegalMove(const char board[CELLS]) const;
};
