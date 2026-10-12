#include "input_player.hpp"

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
    return 0;
}
