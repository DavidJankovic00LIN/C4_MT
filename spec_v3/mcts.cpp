#include "mcts.hpp"

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <vector>

static constexpr int kMctsIterations = 800;
static constexpr double kUctC = 1.41;

static const char kPieces[4] = {PIECE_X, PIECE_O, PIECE_PLUS, PIECE_AT};

static int pieceIndex(char piece) {
    for (int i = 0; i < 4; ++i)
        if (kPieces[i] == piece)
            return i;
    return 0;
}

struct Node {
    int col = 0;
    char playedBy = EMPTY;
    char toMove = EMPTY;
    char winner = EMPTY;
    bool terminal = false;
    int visits = 0;
    double reward[4] = {};
    int untried[BOARD_SIDE_MAX] = {};
    int untriedCount = 0;
    std::vector<Node*> children;
    Node* parent = nullptr;

    Node() = default;
    Node(const Node&) = delete;
    Node& operator=(const Node&) = delete;

    ~Node() {
        for (Node* child : children)
            delete child;
    }
};

static void fillUntried(Node* node, const char board[BOARD_STORAGE_SIZE]) {
    node->untriedCount = 0;
    for (int col = 1; col <= g_n; ++col) {
        if (isLegalMove(board, col))
            node->untried[node->untriedCount++] = col;
    }
}

static int randomLegalColumn(const char board[BOARD_STORAGE_SIZE]) {
    int legal[BOARD_SIDE_MAX];
    int count = 0;
    for (int c = 1; c <= g_n; ++c)
        if (isLegalMove(board, c))
            legal[count++] = c;
    if (count == 0)
        return 0;
    return legal[std::rand() % count];
}

// Nasumična partija do kraja. Vraća figuru pobednika ili EMPTY (nerešeno).
static char playout(char board[BOARD_STORAGE_SIZE], char toMove) {
    while (true) {
        if (isBoardFull(board))
            return EMPTY;
        int col = randomLegalColumn(board);
        if (col == 0)
            return EMPTY;
        int cell = applyMoveCell(board, col, toMove);
        if (cell == 0)
            return EMPTY;
        if (winsFromCell(board, cell))
            return toMove;
        toMove = nextPiece(toMove);
    }
}

static double uct(const Node* parent, const Node* child, char actor) {
    if (child->visits == 0)
        return std::numeric_limits<double>::infinity();
    const double mean = child->reward[pieceIndex(actor)] / static_cast<double>(child->visits);
    return mean + kUctC * std::sqrt(std::log(static_cast<double>(parent->visits))
                                    / static_cast<double>(child->visits));
}

static Node* selectChild(const Node* parent) {
    Node* best = parent->children[0];
    double bestU = uct(parent, best, parent->toMove);
    for (size_t i = 1; i < parent->children.size(); ++i) {
        Node* child = parent->children[i];
        double u = uct(parent, child, parent->toMove);
        if (u > bestU) {
            bestU = u;
            best = child;
        }
    }
    return best;
}

static Node* expand(Node* parent, char work[BOARD_STORAGE_SIZE]) {
    const int col = parent->untried[--parent->untriedCount];
    const int cell = applyMoveCell(work, col, parent->toMove);

    Node* child = new Node();
    child->col = col;
    child->playedBy = parent->toMove;
    child->toMove = nextPiece(parent->toMove);
    child->parent = parent;
    parent->children.push_back(child);

    if (cell != 0 && winsFromCell(work, cell)) {
        child->terminal = true;
        child->winner = child->playedBy;
    } else if (cell == 0 || isBoardFull(work)) {
        child->terminal = true;
        child->winner = EMPTY;
    } else {
        fillUntried(child, work);
    }
    return child;
}

static void backPropagate(Node* node, char winner) {
    while (node != nullptr) {
        node->visits++;
        if (winner != EMPTY) {
            for (int i = 0; i < 4; ++i)
                node->reward[i] += (kPieces[i] == winner) ? 1.0 : -1.0;
        }
        node = node->parent;
    }
}

static int bestRootMove(const Node& root, char me) {
    Node* best = nullptr;
    const int meIdx = pieceIndex(me);
    for (Node* child : root.children) {
        if (child->visits == 0)
            continue;
        if (best == nullptr || child->visits > best->visits) {
            best = child;
            continue;
        }
        if (child->visits == best->visits) {
            const double childAvg = child->reward[meIdx] / static_cast<double>(child->visits);
            const double bestAvg = best->reward[meIdx] / static_cast<double>(best->visits);
            if (childAvg > bestAvg)
                best = child;
        }
    }
    return best != nullptr ? best->col : 0;
}

int chooseColumnMcts(char board[BOARD_STORAGE_SIZE], char me) {
    int win = findWinningColumn(board, me);
    if (win != 0)
        return win;
    int block = findBlockingColumn(board, me);
    if (block != 0)
        return block;

    Node root;
    root.toMove = me;
    fillUntried(&root, board);
    if (root.untriedCount == 0)
        return 0;

    for (int iter = 0; iter < kMctsIterations; ++iter) {
        char work[BOARD_STORAGE_SIZE];
        std::memcpy(work, board, BOARD_STORAGE_SIZE);

        Node* node = &root;
        while (!node->terminal && node->untriedCount == 0 && !node->children.empty()) {
            node = selectChild(node);
            applyMove(work, node->col, node->playedBy);
        }

        if (node->terminal) {
            backPropagate(node, node->winner);
            continue;
        }

        Node* child = expand(node, work);
        if (child->terminal) {
            backPropagate(child, child->winner);
            continue;
        }

        char sim[BOARD_STORAGE_SIZE];
        std::memcpy(sim, work, BOARD_STORAGE_SIZE);
        backPropagate(child, playout(sim, child->toMove));
    }

    return bestRootMove(root, me);
}
