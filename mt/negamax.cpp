#include "negamax.h"

int NegaMaxAI::GetValue(int column)
{
    if (column > 7) return 0;
    int n = 0;
    for (int i = 0; i < 6; i++) {
        if (board[column + 7 * i] == ' ') {
            n = column + 7 * i;
            break;
        }
    }
    if (n > 42) return 0;
    return n;
}

uint8_t NegaMaxAI::winning()
{
    // Horizontalne linije
    for (int row = 0; row < 6; row++) {
        for (int col = 0; col <= 3; col++) {
            char s = board[row * 7 + col];
            if (s != ' ' &&
                s == board[row * 7 + col + 1] &&
                s == board[row * 7 + col + 2] &&
                s == board[row * 7 + col + 3])
                return (s == 'X') ? 1 : 2;
        }
    }
    // Vertikalne linije
    for (int col = 0; col < 7; col++) {
        for (int row = 0; row <= 2; row++) {
            char s = board[row * 7 + col];
            if (s != ' ' &&
                s == board[(row + 1) * 7 + col] &&
                s == board[(row + 2) * 7 + col] &&
                s == board[(row + 3) * 7 + col])
                return (s == 'X') ? 1 : 2;
        }
    }
    // Dijagonala ↘
    for (int row = 0; row <= 2; row++) {
        for (int col = 0; col <= 3; col++) {
            char s = board[row * 7 + col];
            if (s != ' ' &&
                s == board[(row + 1) * 7 + col + 1] &&
                s == board[(row + 2) * 7 + col + 2] &&
                s == board[(row + 3) * 7 + col + 3])
                return (s == 'X') ? 1 : 2;
        }
    }
    // Dijagonala ↙
    for (int row = 0; row <= 2; row++) {
        for (int col = 3; col < 7; col++) {
            char s = board[row * 7 + col];
            if (s != ' ' &&
                s == board[(row + 1) * 7 + col - 1] &&
                s == board[(row + 2) * 7 + col - 2] &&
                s == board[(row + 3) * 7 + col - 3])
                return (s == 'X') ? 1 : 2;
        }
    }
    // Tabla puna?
    for (int i = 0; i < 42; i++)
        if (board[i] == ' ') return 0;
    return 3;
}

int NegaMaxAI::AIManager()
{
    float chance[2] = {9999999, 0};
    for (int column = 1; column <= 7; column++) {
        PlayOut = 0;
        EVA     = 0;
        int PlayNumber = GetValue(column);
        if (PlayNumber != 0) {
            board[PlayNumber] = 'O';
            if (winning() == 2) {
                board[PlayNumber] = ' ';
                return PlayNumber;
            }
            float temp = -(100 * NegaMax(1));
            if (PlayOut != 0)
                temp -= ((100 * EVA) / PlayOut);
            if (-temp >= 100)
                provocation = true;
            if (chance[0] > temp) {
                chance[0] = temp;
                chance[1] = PlayNumber;
            }
            board[PlayNumber] = ' ';
        }
    }
    return chance[1];
}

int NegaMaxAI::NegaMax(int Depth)
{
    char XO;
    int PlayNumber[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    int chance = 0;

    if (Depth % 2 != 0) XO = 'X';
    else                 XO = 'O';

    for (int column = 1; column <= 7; column++)
        PlayNumber[column] = GetValue(column);

    for (int column = 1; column <= 7; column++) {
        if (PlayNumber[column] != 0) {
            board[PlayNumber[column]] = XO;
            if (winning() != 0) {
                PlayOut++;
                if (XO == 'O') EVA++;
                else            EVA--;
                board[PlayNumber[column]] = ' ';
                return -1;
            }
            board[PlayNumber[column]] = ' ';
        }
    }

    if (Depth <= 6) {
        for (int column = 1; column <= 7; column++) {
            int temp = 0;
            if (PlayNumber[column] != 0) {
                board[PlayNumber[column]] = XO;
                if (winning() != 0) {
                    PlayOut++;
                    if (XO == 'O') EVA++;
                    else            EVA--;
                    board[PlayNumber[column]] = ' ';
                    return -1;
                }
                temp = NegaMax(Depth + 1);
                if (column == 1) chance = temp;
                if (chance < temp) chance = temp;
                board[PlayNumber[column]] = ' ';
            }
        }
    }
    return -chance;
}
