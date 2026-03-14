#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <thread>
#include <future>
#include <chrono>
#include <cstring>
#include <cstdint>
#include <cstdlib>
#include <ctime>

#include "negamax.h"
#include "mcts.h"

using namespace std;

// #region agent log
static const char* const DEBUG_LOG_PATH = "/home/david/Desktop/C4_MT/.cursor/debug-06cedf.log";
static void debugLog(const char* location, const char* message, const string& dataJson) {
    ofstream f(DEBUG_LOG_PATH, ios::app);
    if (!f) return;
    long ts = static_cast<long>(time(nullptr));
    f << "{\"sessionId\":\"06cedf\",\"location\":\"" << location << "\",\"message\":\"" << message << "\",\"data\":" << dataJson << ",\"timestamp\":" << ts << "}\n";
}
// #endregion

// ─────────────────────────────────────────────────────────────
//  Result of one complete game
// ─────────────────────────────────────────────────────────────
struct GameResult {
    string  aiName;
    int     winner;      // 1=X(human) won, 2=O(AI) won, 3=tie, 0=unfinished
    char    board[43];   // final board state (main.cpp 1-indexed layout)
    double  durationMs;
};

// ─────────────────────────────────────────────────────────────
//  Read moves from input.txt
// ─────────────────────────────────────────────────────────────
static vector<int> loadMoves(const string& path) {
    vector<int> moves;
    ifstream f(path);
    if (!f.is_open()) {
        cerr << "ERROR: cannot open " << path << endl;
        exit(1);
    }
    int m;
    while (f >> m) moves.push_back(m);
    return moves;
}

// ─────────────────────────────────────────────────────────────
//  Print a board stored in NegaMax 1-indexed layout
//  (index = col + 7*row, col 1-7, row 0-5, row0=top)
// ─────────────────────────────────────────────────────────────
static void printBoard(const char board[43]) {
    cout << "\n    1       2       3       4       5       6       7\n";
    for (int row = 0; row < 6; row++) {
        cout << string(57, '-') << "\n";
        cout << "|";
        for (int col = 1; col <= 7; col++)
            cout << "   " << board[col + 7 * row] << "   |";
        cout << "\n";
    }
    cout << string(57, '-') << "\n";
}

// ─────────────────────────────────────────────────────────────
//  Convert NegaMax board (col 1-7, idx=col+7*row)
//  to MCTS board (col 0-6, idx=row*7+col)
// ─────────────────────────────────────────────────────────────
static void toMctsBoard(const char negaBoard[43], char mctsBoard[CELLS]) {
    for (int row = 0; row < ROWS; ++row)
        for (int col = 0; col < COLS; ++col)
            mctsBoard[row * COLS + col] = negaBoard[(col + 1) + 7 * row];
}

// ─────────────────────────────────────────────────────────────
//  NegaMax game — runs in its own thread
// ─────────────────────────────────────────────────────────────
static GameResult runNegamax(const vector<int>& moves) {
    GameResult res;
    res.aiName = "NegaMax";
    res.winner = 0;
    memset(res.board, ' ', sizeof(res.board));

    NegaMaxAI ai;

    auto t0 = chrono::high_resolution_clock::now();

    size_t moveIdx = 0;
    int turn = 0;

    while (true) {
        turn++;
        // AI plays
        int aiCell = ai.AIManager();
        // #region agent log
        { ostringstream d; d << "{\"game\":\"NegaMax\",\"turn\":" << turn << ",\"aiCell\":" << aiCell << "}"; debugLog("main.cpp:runNegamax_ai", "ai_move", d.str()); }
        // #endregion
        if (aiCell != 0) {
            ai.board[aiCell] = 'O';
            memcpy(res.board, ai.board, sizeof(res.board));
        }

        uint8_t w = ai.winning();
        if (w != 0) { res.winner = w; break; }

        // Human plays from input.txt
        if (moveIdx >= moves.size()) break;
        int col = moves[moveIdx++];
        int cell = ai.GetValue(col);
        if (cell == 0) {
            // skip invalid
            while (moveIdx < moves.size()) {
                col  = moves[moveIdx++];
                cell = ai.GetValue(col);
                if (cell != 0) break;
            }
        }
        // #region agent log
        { ostringstream d; d << "{\"game\":\"NegaMax\",\"turn\":" << turn << ",\"inputCol\":" << col << ",\"cell\":" << cell << "}"; debugLog("main.cpp:runNegamax_human", "human_move", d.str()); }
        // #endregion
        if (cell != 0) {
            ai.board[cell] = 'X';
            memcpy(res.board, ai.board, sizeof(res.board));
        }

        w = ai.winning();
        if (w != 0) { res.winner = w; break; }
    }

    // #region agent log
    { ostringstream d; d << "{\"game\":\"NegaMax\",\"winner\":" << (int)res.winner << "}"; debugLog("main.cpp:runNegamax", "game_end", d.str()); }
    // #endregion
    auto t1 = chrono::high_resolution_clock::now();
    res.durationMs = chrono::duration<double, milli>(t1 - t0).count();
    return res;
}

// ─────────────────────────────────────────────────────────────
//  MCTS GetValue equivalent (1-indexed col, gravity from top)
// ─────────────────────────────────────────────────────────────
static int negaGetValue(const char board[43], int column) {
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

static uint8_t negaWinning(const char board[43]) {
    for (int row = 0; row < 6; row++)
        for (int col = 0; col <= 3; col++) {
            char s = board[row * 7 + col];
            if (s != ' ' &&
                s == board[row * 7 + col + 1] &&
                s == board[row * 7 + col + 2] &&
                s == board[row * 7 + col + 3])
                return (s == 'X') ? 1 : 2;
        }
    for (int col = 0; col < 7; col++)
        for (int row = 0; row <= 2; row++) {
            char s = board[row * 7 + col];
            if (s != ' ' &&
                s == board[(row+1)*7+col] &&
                s == board[(row+2)*7+col] &&
                s == board[(row+3)*7+col])
                return (s == 'X') ? 1 : 2;
        }
    for (int row = 0; row <= 2; row++)
        for (int col = 0; col <= 3; col++) {
            char s = board[row * 7 + col];
            if (s != ' ' &&
                s == board[(row+1)*7+col+1] &&
                s == board[(row+2)*7+col+2] &&
                s == board[(row+3)*7+col+3])
                return (s == 'X') ? 1 : 2;
        }
    for (int row = 0; row <= 2; row++)
        for (int col = 3; col < 7; col++) {
            char s = board[row * 7 + col];
            if (s != ' ' &&
                s == board[(row+1)*7+col-1] &&
                s == board[(row+2)*7+col-2] &&
                s == board[(row+3)*7+col-3])
                return (s == 'X') ? 1 : 2;
        }
    for (int i = 0; i < 42; i++)
        if (board[i] == ' ') return 0;
    return 3;
}

// ─────────────────────────────────────────────────────────────
//  MCTS game — runs in its own thread
// ─────────────────────────────────────────────────────────────
static GameResult runMcts(const vector<int>& moves) {
    GameResult res;
    res.aiName = "MCTS";
    res.winner = 0;
    memset(res.board, ' ', sizeof(res.board));

    auto t0 = chrono::high_resolution_clock::now();

    size_t moveIdx = 0;
    int turn = 0;

    while (true) {
        turn++;
        // Build MCTS board from current res.board state
        char mctsBoard[CELLS];
        toMctsBoard(res.board, mctsBoard);

        MonteCarloTreeSearch mcts(mctsBoard);
        int bestCol = mcts.bestMove(); // 0-indexed

        // Convert back to NegaMax layout; ako MCTS vrati -1 ili kolona puna, fallback na prvu legalnu
        int aiCell = (bestCol >= 0 && bestCol < COLS) ? negaGetValue(res.board, bestCol + 1) : 0;
        if (aiCell == 0) {
            for (int c = 1; c <= 7; c++) {
                aiCell = negaGetValue(res.board, c);
                if (aiCell != 0) break;
            }
        }
        // #region agent log
        { ostringstream d; d << "{\"game\":\"MCTS\",\"turn\":" << turn << ",\"bestCol\":" << bestCol << ",\"aiCell\":" << aiCell << "}"; debugLog("main.cpp:runMcts_ai", "ai_move", d.str()); }
        // #endregion
        if (aiCell != 0)
            res.board[aiCell] = 'O';

        uint8_t w = negaWinning(res.board);
        if (w != 0) { res.winner = w; break; }

        // Human plays from input.txt
        if (moveIdx >= moves.size()) break;
        int col = moves[moveIdx++];
        int cell = negaGetValue(res.board, col);
        if (cell == 0) {
            while (moveIdx < moves.size()) {
                col  = moves[moveIdx++];
                cell = negaGetValue(res.board, col);
                if (cell != 0) break;
            }
        }
        // #region agent log
        { ostringstream d; d << "{\"game\":\"MCTS\",\"turn\":" << turn << ",\"inputCol\":" << col << ",\"cell\":" << cell << "}"; debugLog("main.cpp:runMcts_human", "human_move", d.str()); }
        // #endregion
        if (cell != 0)
            res.board[cell] = 'X';

        w = negaWinning(res.board);
        if (w != 0) { res.winner = w; break; }
    }

    // #region agent log
    { ostringstream d; d << "{\"game\":\"MCTS\",\"winner\":" << res.winner << "}"; debugLog("main.cpp:runMcts", "game_end", d.str()); }
    // #endregion
    auto t1 = chrono::high_resolution_clock::now();
    res.durationMs = chrono::duration<double, milli>(t1 - t0).count();
    return res;
}

// ─────────────────────────────────────────────────────────────
//  Print a single game result
// ─────────────────────────────────────────────────────────────
static void printResult(const GameResult& r) {
    cout << "\n══════════════════════════════════════════════════════\n";
    cout << "  AI: " << r.aiName << "\n";
    cout << "══════════════════════════════════════════════════════\n";

    printBoard(r.board);

    cout << "\n  Rezultat:  ";
    if      (r.winner == 1) cout << "X (Igrač) pobedio\n";
    else if (r.winner == 2) cout << "O (" << r.aiName << " AI) pobedio\n";
    else if (r.winner == 3) cout << "Nerešeno\n";
    else                    cout << "Igra nije završena (nema više poteza)\n";

    cout << "  Trajanje:  " << r.durationMs << " ms\n";
}

// ─────────────────────────────────────────────────────────────
//  main
// ─────────────────────────────────────────────────────────────
int main() {
    const string inputPath = "input.txt";
    vector<int> moves = loadMoves(inputPath);

    cout << "Pokrecemo dve igre paralelno (NegaMax vs MCTS)...\n";
    cout << "Pobuda iz: " << inputPath << " (" << moves.size() << " poteza)\n";

    // Launch both games in separate threads using std::async
    auto futureNega = async(launch::async, runNegamax, cref(moves));
    auto futureMcts = async(launch::async, runMcts,    cref(moves));

    // Wait for both
    GameResult negaResult = futureNega.get();
    GameResult mctsResult = futureMcts.get();

    // Print results
    printResult(negaResult);
    printResult(mctsResult);

    // Comparison summary
    cout << "\n══════════════════════════════════════════════════════\n";
    cout << "  POREDJENJE\n";
    cout << "══════════════════════════════════════════════════════\n";
    cout << "  NegaMax trajanje: " << negaResult.durationMs << " ms\n";
    cout << "  MCTS    trajanje: " << mctsResult.durationMs << " ms\n";

    auto winnerStr = [](const GameResult& r) -> string {
        if (r.winner == 1) return "X (Igrac)";
        if (r.winner == 2) return "O (" + r.aiName + " AI)";
        if (r.winner == 3) return "Nereseno";
        return "Nije zavrseno";
    };

    cout << "  NegaMax ishod:    " << winnerStr(negaResult) << "\n";
    cout << "  MCTS    ishod:    " << winnerStr(mctsResult) << "\n";

    if (negaResult.winner == 2 && mctsResult.winner != 2)
        cout << "\n  -> NegaMax AI bio bolji.\n";
    else if (mctsResult.winner == 2 && negaResult.winner != 2)
        cout << "\n  -> MCTS AI bio bolji.\n";
    else if (negaResult.winner == 2 && mctsResult.winner == 2) {
        cout << "\n  -> Oba AI pobedila. Brzi: ";
        cout << (negaResult.durationMs < mctsResult.durationMs ? "NegaMax" : "MCTS") << ".\n";
    } else {
        cout << "\n  -> Ni jedan AI nije pobedio.\n";
    }

    return 0;
}
