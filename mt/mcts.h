#pragma once

#include "board.h"
#include <vector>
#include <cmath>
#include <cstdint>
#include <limits>

static constexpr char EMPTY = ' ';
static constexpr char AI    = 'O';
static constexpr char HUMAN = 'X';

int  getLowestFreeCell(const char board[BOARD_STORAGE_SIZE], int col1);
bool isLegalMove(const char board[BOARD_STORAGE_SIZE], int col1);
bool applyMove(char board[BOARD_STORAGE_SIZE], int col1, char piece);
char findWinner(const char board[BOARD_STORAGE_SIZE]);
bool isBoardFull(const char board[BOARD_STORAGE_SIZE]);

struct Node {
    int    col;
    double score;
    int    visits;
    Node*  parent;
    std::vector<Node*> childArray;

    explicit Node(int col = -1, Node* parent = nullptr);
    ~Node();

    Node(const Node&)            = delete;
    Node& operator=(const Node&) = delete;
};

class MonteCarloTreeSearch {
public:
    MonteCarloTreeSearch(const char currentBoard[BOARD_STORAGE_SIZE], int iterations = 50000);
    ~MonteCarloTreeSearch();

    int bestMove();

    void setIterations(int n) { iterations_ = n; }

    bool finalMove;

private:
    char board_[BOARD_STORAGE_SIZE];
    int  iterations_;
    static constexpr int SIMS_PER_EXPAND = 100;

    Node* rootNode_;

    void getBoardAtNode(Node* node, char out[BOARD_STORAGE_SIZE], bool* turnIsAI) const;
    double getUCT(Node* child) const;
    Node*  getBestChild(Node* parent) const;

    void runIteration();
    void expand(Node* leaf);
    double simulate(char board[BOARD_STORAGE_SIZE], bool turnIsAI) const;
    void   backPropagate(Node* node, double value);

    int randomLegalMove(const char board[BOARD_STORAGE_SIZE]) const;
};
