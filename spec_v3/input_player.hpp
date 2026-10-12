#pragma once

#include "board.hpp"

#include <string>
#include <vector>

class InputPlayer {
public:
    explicit InputPlayer(const std::string& path);

    bool loaded() const { return loaded_; }
    bool hasMoves() const { return index_ < moves_.size(); }

    // Vraća 1-based legalnu kolonu sa liste, ili 0 ako nema više poteza.
    int chooseColumn(char board[BOARD_STORAGE_SIZE]);

private:
    std::vector<int> moves_;
    size_t           index_  = 0;
    bool             loaded_ = false;
};
