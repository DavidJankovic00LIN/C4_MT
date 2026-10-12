#include "draw_board.hpp"

#include <iostream>
#include <string>

void drawBoard(const char* board) {
    if (board == nullptr || g_n <= 0) {
        std::cerr << "drawBoard: neispravni argumenti\n";
        return;
    }

    std::cout << "\n";
    for (int col = 1; col <= g_n; ++col)
        std::cout << "    " << col << "   ";
    std::cout << "\n";

    const int lineLen = g_n * 8 + 1;
    for (int row = 0; row < g_n; ++row) {
        std::cout << std::string(lineLen, '-') << "\n";
        std::cout << "|";
        for (int col1 = 1; col1 <= g_n; ++col1) {
            char cell = board[cellIndex(col1, row)];
            if (cell == '\0')
                cell = EMPTY;
            std::cout << "   " << cell << "   |";
        }
        std::cout << "\n";
    }
    std::cout << std::string(lineLen, '-') << "\n";
}
