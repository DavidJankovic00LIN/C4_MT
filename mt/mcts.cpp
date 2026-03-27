#include "mcts.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <iostream>

int getLowestFreeCell(const char board[BOARD_STORAGE_SIZE], int col1) {
    return lowestFreeCell(board, col1);
}

bool isLegalMove(const char board[BOARD_STORAGE_SIZE], int col1) {
    return isLegalMoveCol1(board, col1);
}

bool applyMove(char board[BOARD_STORAGE_SIZE], int col1, char piece) {
    return applyMoveCol1(board, col1, piece);
}

char findWinner(const char board[BOARD_STORAGE_SIZE]) {
    return boardFindWinner(board);
}

bool isBoardFull(const char board[BOARD_STORAGE_SIZE]) {
    return boardIsFull(board);
}

Node::Node(int col, Node* parent)
    : col(col), score(0.0), visits(0), parent(parent) {}

Node::~Node() {
    for (Node* child : childArray)
        delete child;
}

MonteCarloTreeSearch::MonteCarloTreeSearch(const char currentBoard[BOARD_STORAGE_SIZE], int iterations)
    : finalMove(false), iterations_(iterations), rootNode_(nullptr) {
    std::memcpy(board_, currentBoard, BOARD_STORAGE_SIZE);
    std::srand(static_cast<unsigned>(std::time(nullptr)));
}

MonteCarloTreeSearch::~MonteCarloTreeSearch() {
    delete rootNode_;
    rootNode_ = nullptr;
}

void MonteCarloTreeSearch::getBoardAtNode(Node* node, char out[BOARD_STORAGE_SIZE], bool* turnIsAI) const {
    std::vector<Node*> path;
    for (Node* p = node; p != nullptr; p = p->parent)
        path.push_back(p);
    std::memcpy(out, board_, BOARD_STORAGE_SIZE);
    bool turn = true;
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
    if (p == nullptr || p->visits == 0)
        return child->score / static_cast<double>(child->visits);
    return child->score / static_cast<double>(child->visits)
           + 1.41 * std::sqrt(std::log(static_cast<double>(p->visits)) / static_cast<double>(child->visits));
}

Node* MonteCarloTreeSearch::getBestChild(Node* parent) const {
    if (parent->childArray.empty())
        return nullptr;
    Node* best     = parent->childArray[0];
    double bestUct = getUCT(best);
    for (Node* child : parent->childArray) {
        double u = getUCT(child);
        if (u > bestUct) {
            bestUct = u;
            best    = child;
        }
    }
    return best;
}

void MonteCarloTreeSearch::runIteration() {
    Node* current = rootNode_;
    while (!current->childArray.empty()) {
        current = getBestChild(current);
        if (current == nullptr)
            return;
    }
    expand(current);
    if (current->childArray.empty())
        return;
    Node* child = current->childArray[0];
    char simBoard[BOARD_STORAGE_SIZE];
    bool turnIsAI;
    getBoardAtNode(child, simBoard, &turnIsAI);
    for (int j = 0; j < SIMS_PER_EXPAND; ++j) {
        double value = simulate(simBoard, turnIsAI);
        backPropagate(child, value);
    }
}

void MonteCarloTreeSearch::expand(Node* leaf) {
    char state[BOARD_STORAGE_SIZE];
    bool turnIsAI;
    getBoardAtNode(leaf, state, &turnIsAI);
    for (int c = 1; c <= g_board_cols; ++c) {
        if (!isLegalMove(state, c))
            continue;
        leaf->childArray.push_back(new Node(c, leaf));
    }
}

double MonteCarloTreeSearch::simulate(char board[BOARD_STORAGE_SIZE], bool turnIsAI) const {
    char sim[BOARD_STORAGE_SIZE];
    std::memcpy(sim, board, BOARD_STORAGE_SIZE);
    bool turn = turnIsAI;
    while (findWinner(sim) == EMPTY && !isBoardFull(sim)) {
        int col1 = randomLegalMove(sim);
        if (col1 < 0)
            break;
        applyMove(sim, col1, turn ? AI : HUMAN);
        turn = !turn;
    }
    char winner = findWinner(sim);
    if (winner == AI)
        return 1.0;
    if (winner == HUMAN)
        return -2.0;
    return 0.0;
}

void MonteCarloTreeSearch::backPropagate(Node* node, double value) {
    Node* n = node;
    while (n != nullptr) {
        n->visits++;
        n->score += value;
        n = n->parent;
    }
}

int MonteCarloTreeSearch::bestMove() {
    delete rootNode_;
    rootNode_ = new Node(-1, nullptr);

    for (int i = 0; i < iterations_; ++i)
        runIteration();

    if (!rootNode_ || rootNode_->childArray.empty()) {
        for (int c = 1; c <= g_board_cols; ++c)
            if (isLegalMove(board_, c))
                return c;
        return -1;
    }

    Node* best       = nullptr;
    double bestScore = -std::numeric_limits<double>::infinity();
    for (Node* child : rootNode_->childArray) {
        if (!isLegalMove(board_, child->col))
            continue;
        double s = child->score;
        if (child->visits > 0 && s > bestScore) {
            bestScore = s;
            best      = child;
        }
    }
    if (best)
        return best->col;

    for (Node* child : rootNode_->childArray) {
        if (isLegalMove(board_, child->col))
            return child->col;
    }
    for (int c = 1; c <= g_board_cols; ++c)
        if (isLegalMove(board_, c))
            return c;
    return -1;
}

int MonteCarloTreeSearch::randomLegalMove(const char board[BOARD_STORAGE_SIZE]) const {
    int legal[BOARD_COLS_MAX];
    int count = 0;
    for (int c = 1; c <= g_board_cols; ++c)
        if (isLegalMove(board, c))
            legal[count++] = c;
    if (count == 0)
        return -1;
    return legal[std::rand() % count];
}
