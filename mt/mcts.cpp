#include "mcts.h"

#include <cmath>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <limits>

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
    : col(col),
      score(0.0),
      visits(0),
      parent(parent),
      untriedCount(0),
      terminal(false),
      result(0.0),
      aiToMove(true) {}

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

static void fillUntried(Node* node, const char board[BOARD_STORAGE_SIZE]) {
    node->untriedCount = 0;
    for (int col = 1; col <= g_board_cols; ++col) {
        if (isLegalMove(board, col))
            node->untried[node->untriedCount++] = col;
    }
}

namespace {

struct Scratch {
    char work[BOARD_STORAGE_SIZE];
    int  placed[BOARD_ROWS_MAX * BOARD_COLS_MAX];
    int  n;

    explicit Scratch(const char* src) : n(0) {
        std::memcpy(work, src, sizeof(work));
    }

    int place(int col, char piece) {
        int cell = lowestFreeCell(work, col);
        if (cell == 0)
            return 0;
        work[cell] = piece;
        placed[n++] = cell;
        return cell;
    }

    ~Scratch() {
        for (int i = n - 1; i >= 0; --i)
            work[placed[i]] = ' ';
    }
};

}  // namespace

double MonteCarloTreeSearch::simulate(char board[BOARD_STORAGE_SIZE], bool turnIsAI) const {
    bool turn = turnIsAI;
    while (!isBoardFull(board)) {
        int col = randomLegalMove(board);
        if (col < 0)
            return 0.0;
        int cell = lowestFreeCell(board, col);
        if (cell == 0)
            return 0.0;
        board[cell] = turn ? AI : HUMAN;
        if (winsFromCell(board, cell))
            return turn ? 1.0 : -2.0;
        turn = !turn;
    }
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

void MonteCarloTreeSearch::runIteration() {
    Scratch scratch(board_);

    Node* node = rootNode_;
    while (!node->terminal && node->untriedCount == 0 && !node->childArray.empty()) {
        node = getBestChild(node);
        if (node == nullptr)
            return;
        const char piece = node->parent->aiToMove ? AI : HUMAN;
        if (scratch.place(node->col, piece) == 0)
            return;
    }

    if (node->terminal) {
        backPropagate(node, node->result);
        return;
    }
    if (node->untriedCount == 0) {
        backPropagate(node, 0.0);
        return;
    }

    const int col = node->untried[--node->untriedCount];
    const char piece = node->aiToMove ? AI : HUMAN;
    const int cell = scratch.place(col, piece);
    Node* child = new Node(col, node);
    child->aiToMove = !node->aiToMove;
    node->childArray.push_back(child);

    if (cell != 0 && winsFromCell(scratch.work, cell)) {
        child->terminal = true;
        child->result = (piece == AI) ? 1.0 : -2.0;
        backPropagate(child, child->result);
        return;
    }
    if (cell == 0 || isBoardFull(scratch.work)) {
        child->terminal = true;
        child->result = 0.0;
        backPropagate(child, 0.0);
        return;
    }

    fillUntried(child, scratch.work);
    backPropagate(child, simulate(scratch.work, child->aiToMove));
}

int MonteCarloTreeSearch::bestMove() {
    delete rootNode_;
    rootNode_ = new Node(-1, nullptr);
    rootNode_->aiToMove = true;
    fillUntried(rootNode_, board_);

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
