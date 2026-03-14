#include "mcts.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <iostream>

// ═══════════════════════════════════════════════════════════════
//  Board helpers
// ═══════════════════════════════════════════════════════════════

/*
 * The board is stored as a flat array of 42 chars (row-major, top row first).
 * index = row * COLS + col   (row 0 = top, row 5 = bottom)
 *
 * Gravity: a piece falls to the *lowest* free row in the chosen column,
 * i.e. the highest row index that is still EMPTY.
 */

int getLowestFreeCell(const char board[CELLS], int col) {
    if (col < 0 || col >= COLS) return 0;
    for (int row = ROWS - 1; row >= 0; --row) {
        if (board[row * COLS + col] == EMPTY)
            return row * COLS + col;
    }
    return 0; // column full
}

bool isLegalMove(const char board[CELLS], int col) {
    if (col < 0 || col >= COLS) return false;
    // Top cell of column must be empty
    return board[col] == EMPTY;
}

bool applyMove(char board[CELLS], int col, char piece) {
    int cell = getLowestFreeCell(board, col);
    if (cell == 0 && board[col] != EMPTY) return false;
    board[cell] = piece;
    return true;
}

char findWinner(const char board[CELLS]) {
    // Horizontal
    for (int r = 0; r < ROWS; ++r)
        for (int c = 0; c <= COLS - 4; ++c) {
            char s = board[r * COLS + c];
            if (s != EMPTY &&
                s == board[r * COLS + c + 1] &&
                s == board[r * COLS + c + 2] &&
                s == board[r * COLS + c + 3])
                return s;
        }

    // Vertical
    for (int c = 0; c < COLS; ++c)
        for (int r = 0; r <= ROWS - 4; ++r) {
            char s = board[r * COLS + c];
            if (s != EMPTY &&
                s == board[(r + 1) * COLS + c] &&
                s == board[(r + 2) * COLS + c] &&
                s == board[(r + 3) * COLS + c])
                return s;
        }

    // Diagonal ↘
    for (int r = 0; r <= ROWS - 4; ++r)
        for (int c = 0; c <= COLS - 4; ++c) {
            char s = board[r * COLS + c];
            if (s != EMPTY &&
                s == board[(r + 1) * COLS + c + 1] &&
                s == board[(r + 2) * COLS + c + 2] &&
                s == board[(r + 3) * COLS + c + 3])
                return s;
        }

    // Diagonal ↙
    for (int r = 0; r <= ROWS - 4; ++r)
        for (int c = 3; c < COLS; ++c) {
            char s = board[r * COLS + c];
            if (s != EMPTY &&
                s == board[(r + 1) * COLS + c - 1] &&
                s == board[(r + 2) * COLS + c - 2] &&
                s == board[(r + 3) * COLS + c - 3])
                return s;
        }

    return EMPTY;
}

bool isBoardFull(const char board[CELLS]) {
    for (int i = 0; i < CELLS; ++i)
        if (board[i] == EMPTY) return false;
    return true;
}


// ═══════════════════════════════════════════════════════════════
//  Node
// ═══════════════════════════════════════════════════════════════

Node::Node(int col, Node* parent)
    : col(col), score(0.0), visits(0), parent(parent) {}

Node::~Node() {
    for (Node* child : childArray)
        delete child;
}


// ═══════════════════════════════════════════════════════════════
//  MonteCarloTreeSearch
// ═══════════════════════════════════════════════════════════════

MonteCarloTreeSearch::MonteCarloTreeSearch(const char currentBoard[CELLS], int iterations)
    : finalMove(false), iterations_(iterations), rootNode_(nullptr) {
    std::memcpy(board_, currentBoard, CELLS);
    std::srand(static_cast<unsigned>(std::time(nullptr)));
}

MonteCarloTreeSearch::~MonteCarloTreeSearch() {
    delete rootNode_;
    rootNode_ = nullptr;
}

// ── Board-at-node and UCT (src-style) ─────────────────────────

void MonteCarloTreeSearch::getBoardAtNode(Node* node, char out[CELLS], bool* turnIsAI) const {
    std::vector<Node*> path;
    for (Node* p = node; p != nullptr; p = p->parent)
        path.push_back(p);
    std::memcpy(out, board_, CELLS);
    bool turn = true;  // at root it's AI's turn
    for (int i = static_cast<int>(path.size()) - 2; i >= 0; --i) {
        Node* n = path[i];
        applyMove(out, n->col, turn ? AI : HUMAN);
        turn = !turn;
    }
    *turnIsAI = turn;
}

double MonteCarloTreeSearch::getUCT(Node* child) const {
    if (child->visits == 0)
        return std::numeric_limits<double>::infinity();
    Node* p = child->parent;
    if (p == nullptr || p->visits == 0) return child->score / static_cast<double>(child->visits);
    return child->score / static_cast<double>(child->visits)
           + 1.41 * std::sqrt(std::log(static_cast<double>(p->visits)) / static_cast<double>(child->visits));
}

Node* MonteCarloTreeSearch::getBestChild(Node* parent) const {
    if (parent->childArray.empty()) return nullptr;
    Node* best = parent->childArray[0];
    double bestUct = getUCT(best);
    for (Node* child : parent->childArray) {
        double u = getUCT(child);
        if (u > bestUct) {
            bestUct = u;
            best = child;
        }
    }
    return best;
}

void MonteCarloTreeSearch::runIteration() {
    Node* current = rootNode_;
    while (!current->childArray.empty()) {
        current = getBestChild(current);
        if (current == nullptr) return;
    }
    expand(current);
    if (current->childArray.empty()) return;
    Node* child = current->childArray[0];
    char simBoard[CELLS];
    bool turnIsAI;
    getBoardAtNode(child, simBoard, &turnIsAI);
    for (int j = 0; j < SIMS_PER_EXPAND; ++j) {
        double value = simulate(simBoard, turnIsAI);
        backPropagate(child, value);
    }
}

void MonteCarloTreeSearch::expand(Node* leaf) {
    char state[CELLS];
    bool turnIsAI;
    getBoardAtNode(leaf, state, &turnIsAI);
    for (int c = 0; c < COLS; ++c) {
        if (!isLegalMove(state, c)) continue;
        leaf->childArray.push_back(new Node(c, leaf));
    }
}

double MonteCarloTreeSearch::simulate(char board[CELLS], bool turnIsAI) const {
    char sim[CELLS];
    std::memcpy(sim, board, CELLS);
    bool turn = turnIsAI;
    while (findWinner(sim) == EMPTY && !isBoardFull(sim)) {
        int col = randomLegalMove(sim);
        if (col < 0) break;
        applyMove(sim, col, turn ? AI : HUMAN);
        turn = !turn;
    }
    char winner = findWinner(sim);
    if (winner == AI)   return 1.0;
    if (winner == HUMAN) return -2.0;
    return 0.0;  // draw
}

void MonteCarloTreeSearch::backPropagate(Node* node, double value) {
    Node* n = node;
    while (n != nullptr) {
        n->visits++;
        n->score += value;
        n = n->parent;
    }
}

// ── Public entry point ────────────────────────────────────────

int MonteCarloTreeSearch::bestMove() {
    delete rootNode_;
    rootNode_ = new Node(-1, nullptr);

    for (int i = 0; i < iterations_; ++i)
        runIteration();

    if (!rootNode_ || rootNode_->childArray.empty()) {
        for (int c = 0; c < COLS; ++c)
            if (isLegalMove(board_, c)) return c;
        return -1;
    }

    Node* best = nullptr;
    double bestScore = -std::numeric_limits<double>::infinity();
    for (Node* child : rootNode_->childArray) {
        if (!isLegalMove(board_, child->col)) continue;
        double s = child->score;
        if (child->visits > 0 && s > bestScore) {
            bestScore = s;
            best = child;
        }
    }
    if (best) return best->col;

    for (Node* child : rootNode_->childArray) {
        if (isLegalMove(board_, child->col)) return child->col;
    }
    for (int c = 0; c < COLS; ++c)
        if (board_[c] == EMPTY) return c;
    return -1;
}

// ── Utility ───────────────────────────────────────────────────

int MonteCarloTreeSearch::randomLegalMove(const char board[CELLS]) const {
    int legal[COLS];
    int count = 0;
    for (int c = 0; c < COLS; ++c)
        if (isLegalMove(board, c))
            legal[count++] = c;
    if (count == 0) return -1;
    return legal[std::rand() % count];
}