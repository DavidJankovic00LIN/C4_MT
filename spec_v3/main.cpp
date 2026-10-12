#include "board.hpp"
#include "draw_board.hpp"
#include "paranoid_negamax.hpp"
#include "flat_mcts.hpp"
#include "greedy.hpp"
#include "input_player.hpp"

#include <ctime>
#include <iostream>
#include <limits>
#include <string>

static const char* pieceName(char p) {
    switch (p) {
    case PIECE_X:    return "X (Paranoid NegaMax)";
    case PIECE_O:    return "O (Flat MCTS)";
    case PIECE_PLUS: return "+ (Greedy)";
    case PIECE_AT:   return "@ (input.txt)";
    default:         return "?";
    }
}

static void promptBoardSize() {
    std::cout << "Kvadratna tabla N x N. Unesite N (" << kBoardSideMin << "-"
              << kBoardSideMax << "): ";
    int n = 0;
    for (;;) {
        if (!(std::cin >> n)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Neispravan unos. Unesite N (" << kBoardSideMin << "-"
                      << kBoardSideMax << "): ";
            continue;
        }
        if (boardSetSize(n)) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Tabla: " << g_n << " x " << g_n << ".\n";
            return;
        }
        std::cout << "N mora biti " << kBoardSideMin << "-" << kBoardSideMax << ". Ponovo: ";
    }
}

int main() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    promptBoardSize();

    char board[BOARD_STORAGE_SIZE];
    clearBoard(board);

    InputPlayer inputPlayer("input.txt");
    if (!inputPlayer.loaded()) {
        std::cerr << "Igra zahteva input.txt za igraca @.\n";
        return 1;
    }

    std::cout << "Igraci: X=ParanoidNegaMax, O=FlatMCTS, +=Greedy, @=input.txt\n";
    std::cout << "Redosled: X -> O -> + -> @\n";

    drawBoard(board);

    char turn = PIECE_X;
    while (true) {
        int col = 0;
        if (turn == PIECE_X)
            col = chooseColumnParanoidNegamax(board, PIECE_X);
        else if (turn == PIECE_O)
            col = chooseColumnFlatMcts(board, PIECE_O);
        else if (turn == PIECE_PLUS)
            col = chooseColumnGreedy(board, PIECE_PLUS);
        else
            col = inputPlayer.chooseColumn(board);

        if (col == 0) {
            if (turn == PIECE_AT)
                std::cout << "\nNema vise poteza u input.txt. Igra prekinuta.\n";
            else
                std::cout << "\nIgrac " << turn << " nema legalan potez. Igra prekinuta.\n";
            break;
        }

        if (!applyMove(board, col, turn)) {
            std::cout << "\nGreska: applyMove failed za kolonu " << col << ".\n";
            break;
        }

        std::cout << "\nPotez " << pieceName(turn) << " -> kolona " << col << "\n";
        drawBoard(board);

        char winner = findWinner(board);
        if (winner != EMPTY) {
            std::cout << "\nPobednik: " << pieceName(winner) << "\n";
            return 0;
        }
        if (isBoardFull(board)) {
            std::cout << "\nNereseno (tabla puna).\n";
            return 0;
        }

        turn = nextPiece(turn);
    }

    return 0;
}
