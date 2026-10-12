#include "input_player.hpp"

#include <cstdlib>
#include <fstream>
#include <iostream>

InputPlayer::InputPlayer(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) {
        std::cerr << "ERROR: ne mogu da otvorim " << path << "\n";
        loaded_ = false;
        return;
    }
    int m;
    while (f >> m)
        moves_.push_back(m);
    loaded_ = true;
    index_  = 0;
}

int InputPlayer::chooseColumn(char board[BOARD_STORAGE_SIZE]) {
    while (index_ < moves_.size()) {
        int col = moves_[index_++];
        if (isLegalMove(board, col))
            return col;
        std::cout << "WARNING: nelegalan potez " << col << ", uzimam sledeci.\n";
    }

    // Fallback: nasumična legalna kolona da partija može da se završi
    int legal[BOARD_SIDE_MAX];
    int count = 0;
    for (int c = 1; c <= g_n; ++c)
        if (isLegalMove(board, c))
            legal[count++] = c;
    if (count == 0)
        return 0;
    std::cout << "WARNING: input.txt potrosen, igram nasumicnu legalnu kolonu.\n";
    return legal[std::rand() % count];
}
