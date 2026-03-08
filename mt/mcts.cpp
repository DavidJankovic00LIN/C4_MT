#include "mcts.h"

#include <algorithm>
#include <cassert>
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
    : col(col), wins(0), moves(0), visits(0), ucbValue(0.0),
      parent(parent) {}

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

// ── Public entry point ────────────────────────────────────────

int MonteCarloTreeSearch::bestMove() {
    delete rootNode_;
    rootNode_    = new Node();
    maxUcbNodes_ = {};
    finalMove    = false;

    for (int i = 0; i < iterations_; ++i)
        select(rootNode_);

    // Guard: if no legal moves were found, retry (mirrors Java fallback)
    if (maxUcbNodes_.empty()) return bestMove();

    int idx;
    do {
        idx = std::rand() % static_cast<int>(maxUcbNodes_.size());
    } while (idx == static_cast<int>(maxUcbNodes_.size())); // safety (always false — mirrors Java)

    return maxUcbNodes_[idx]->col;
}

// ── Selection phase ───────────────────────────────────────────

/*
 * If the parent has no children yet  → expand it.
 * If any child has never been visited → rollout that child.
 * Otherwise pick the child with the highest UCB and recurse or expand.
 */
void MonteCarloTreeSearch::select(Node* parentNode) {
    std::vector<Node*>& children = parentNode->childArray;

    if (!children.empty()) {
        // Roll out the first unvisited child
        for (Node* child : children) {
            if (child->visits == 0) {
                rollOut(child);
                return;
            }
        }

        // All children visited — pick the one with the highest UCB value
        Node* best = children[0];
        for (Node* child : children) {
            if (child->ucbValue > best->ucbValue)
                best = child;
        }

        if (best->childArray.empty())
            expand(best);
        else
            select(best);

    } else {
        expand(parentNode);
    }
}

// ── Expansion phase ───────────────────────────────────────────

/*
 * Add one child node per column (0 … COLS-1), then rollout the first child.
 */
void MonteCarloTreeSearch::expand(Node* parentNode) {
    for (int col = 0; col < COLS; ++col)
        parentNode->childArray.push_back(new Node(col, parentNode));

    rollOut(parentNode->childArray[0]);
}

// ── Rollout phase ─────────────────────────────────────────────

/*
 * Simulate a random game from the current board state.
 * AI plays first in the simulation (mirrors Java: first switch → GREEN/AI).
 * Scoring:
 *   AI wins  → wins = 2
 *   Draw     → wins = 1
 *   Human wins → wins = 0, moves = 0
 */
void MonteCarloTreeSearch::rollOut(Node* nonVisitedNode) {
    char simBoard[CELLS];
    std::memcpy(simBoard, board_, CELLS);

    int  wins         = 0;
    int  moves        = 0;
    char currentPiece = HUMAN; // will be flipped to AI on the first iteration
    bool firstTime    = true;

    char winner = EMPTY;
    while ((winner = findWinner(simBoard)) == EMPTY && !isBoardFull(simBoard)) {
        // Alternate pieces
        currentPiece = (currentPiece == AI) ? HUMAN : AI;

        int chosenCol;
        if (firstTime) {
            // Play the node's column as the first move of the simulation
            chosenCol = nonVisitedNode->col;
            firstTime = false;
            // If that column is already full, fall back to a random legal move
            if (!isLegalMove(simBoard, chosenCol)) {
                chosenCol = randomLegalMove(simBoard);
                if (chosenCol == -1) break; // board full
            }
        } else {
            chosenCol = randomLegalMove(simBoard);
            if (chosenCol == -1) break;
        }

        applyMove(simBoard, chosenCol, currentPiece);
        if (currentPiece == AI) ++moves;
    }

    if      (winner == AI)    { wins = 2; }
    else if (winner == HUMAN) { wins = 0; moves = 0; }
    else                      { wins = 1; }

    // Forced win detection: AI wins in exactly one move
    if (winner == AI && moves == 1)
        finalMove = true;

    backPropagate(nonVisitedNode, wins, moves);
}

// ── Back-propagation phase ────────────────────────────────────

/*
 * Walk from rolledOutNode to the root, accumulating statistics.
 * Then rebuild maxUcbNodes_ by traversing the entire tree.
 */
void MonteCarloTreeSearch::backPropagate(Node* rolledOutNode, int win, int moves) {
    Node* node = rolledOutNode;
    node->wins   += win;
    node->moves  += moves;
    node->visits += 1;

    while (node->parent != nullptr) {
        node = node->parent;
        node->wins   += win;
        node->moves  += moves;
        node->visits += 1;
    }

    maxUcbNodes_.clear();
    updateMaxUcbNode(rootNode_);
}

// ── UCB update ────────────────────────────────────────────────

/*
 * Traverse the tree and maintain the list of nodes that share the
 * current highest UCB value (ties broken by fewer moves).
 *
 * UCB1 formula:
 *   exploitation = wins / visits
 *   exploration  = sqrt(2) * sqrt( ln(parent.visits) / visits )
 */
void MonteCarloTreeSearch::updateMaxUcbNode(Node* node) {
    if (node == rootNode_ && node->childArray.empty()) return;

    for (Node* child : node->childArray) {
        if (child->visits > 0) {
            double exploitation = child->wins / static_cast<double>(child->visits);
            double exploration  = std::sqrt(2.0) *
                                  std::sqrt(std::log(static_cast<double>(child->parent->visits)) /
                                            static_cast<double>(child->visits));
            child->ucbValue = exploitation + exploration;

            if (isLegalMove(board_, child->col) && child->ucbValue > 0.0) {
                if (maxUcbNodes_.empty()) {
                    maxUcbNodes_.push_back(child);
                } else {
                    double bestUcb   = maxUcbNodes_[0]->ucbValue;
                    int    bestMoves = maxUcbNodes_[0]->moves;

                    // Use a small epsilon for floating-point comparison
                    constexpr double EPS = 1e-9;

                    if (child->ucbValue > bestUcb + EPS) {
                        // Strictly better
                        maxUcbNodes_.clear();
                        maxUcbNodes_.push_back(child);
                    } else if (child->ucbValue >= bestUcb - EPS) {
                        // Tied UCB — prefer fewer moves (faster win)
                        if (child->moves < bestMoves) {
                            maxUcbNodes_.clear();
                            maxUcbNodes_.push_back(child);
                        } else if (child->moves == bestMoves) {
                            maxUcbNodes_.push_back(child);
                        }
                    }
                }
            }

            // Recurse into children
            if (!child->childArray.empty())
                updateMaxUcbNode(child);
        }
    }
}

// ── Utility ───────────────────────────────────────────────────

int MonteCarloTreeSearch::randomLegalMove(const char board[CELLS]) const {
    // Collect all legal columns and pick one at random
    int legal[COLS];
    int count = 0;
    for (int c = 0; c < COLS; ++c)
        if (isLegalMove(board, c))
            legal[count++] = c;

    if (count == 0) return -1;
    return legal[std::rand() % count];
}
