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
#include <limits>

#include "board.h"
#include "negamax.h"
#include "mcts.h"

using namespace std;

// #region agent log
static const char* const DEBUG_LOG_PATH = "/home/david/Desktop/C4_MT/.cursor/debug-06cedf.log";
static void debugLog(const char* location, const char* message, const string& dataJson) {
    ofstream f(DEBUG_LOG_PATH, ios::app);
    if (!f) return;
    long ts = static_cast<long>(time(nullptr));
    f << "{\"sessionId\":\"06cedf\",\"location\":\"" << location << "\",\"message\":\"" << message
      << "\",\"data\":" << dataJson << ",\"timestamp\":" << ts << "}\n";
}
// #endregion

struct GameResult {
    string  aiName;
    int     winner;
    char    board[BOARD_STORAGE_SIZE];
    double  durationMs;
};

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

static void printBoard(const char board[BOARD_STORAGE_SIZE]) {
    cout << "\n";
    for (int col1 = 1; col1 <= g_board_cols; ++col1)
        cout << "    " << col1 << "   ";
    cout << "\n";
    const int lineLen = g_board_cols * 8 + 1;
    for (int row = 0; row < g_board_rows; row++) {
        cout << string(lineLen, '-') << "\n";
        cout << "|";
        for (int col1 = 1; col1 <= g_board_cols; col1++)
            cout << "   " << board[cellIndex(col1, row)] << "   |";
        cout << "\n";
    }
    cout << string(lineLen, '-') << "\n";
}

static uint8_t negaWinningFromBoard(const char b[BOARD_STORAGE_SIZE]) {
    char w = boardFindWinner(b);
    if (w == 'X')
        return 1;
    if (w == 'O')
        return 2;
    if (boardIsFull(b))
        return 3;
    return 0;
}

static GameResult runNegamax(const vector<int>& moves) {
    GameResult res;
    res.aiName = "NegaMax";
    res.winner = 0;
    memset(res.board, ' ', sizeof(res.board));

    NegaMaxAI ai;

    auto t0 = chrono::high_resolution_clock::now();

    size_t moveIdx = 0;
    int    turn     = 0;

    while (true) {
        turn++;
        int aiCell = ai.AIManager();
        // #region agent log
        {
            ostringstream d;
            d << "{\"game\":\"NegaMax\",\"turn\":" << turn << ",\"aiCell\":" << aiCell << "}";
            debugLog("main.cpp:runNegamax_ai", "ai_move", d.str());
        }
        // #endregion
        if (aiCell != 0) {
            ai.board[aiCell] = 'O';
            memcpy(res.board, ai.board, sizeof(res.board));
        }

        uint8_t w = ai.winning();
        if (w != 0) {
            res.winner = w;
            break;
        }

        if (moveIdx >= moves.size()) break;
        int col  = moves[moveIdx++];
        int cell = ai.GetValue(col);
        if (cell == 0) {
            while (moveIdx < moves.size()) {
                col  = moves[moveIdx++];
                cell = ai.GetValue(col);
                if (cell != 0) break;
            }
        }
        // #region agent log
        {
            ostringstream d;
            d << "{\"game\":\"NegaMax\",\"turn\":" << turn << ",\"inputCol\":" << col << ",\"cell\":" << cell << "}";
            debugLog("main.cpp:runNegamax_human", "human_move", d.str());
        }
        // #endregion
        if (cell != 0) {
            ai.board[cell] = 'X';
            memcpy(res.board, ai.board, sizeof(res.board));
        }

        w = ai.winning();
        if (w != 0) {
            res.winner = w;
            break;
        }
    }

    // #region agent log
    {
        ostringstream d;
        d << "{\"game\":\"NegaMax\",\"winner\":" << (int)res.winner << "}";
        debugLog("main.cpp:runNegamax", "game_end", d.str());
    }
    // #endregion
    auto t1          = chrono::high_resolution_clock::now();
    res.durationMs = chrono::duration<double, milli>(t1 - t0).count();
    return res;
}

static GameResult runMcts(const vector<int>& moves) {
    GameResult res;
    res.aiName = "MCTS";
    res.winner = 0;
    memset(res.board, ' ', sizeof(res.board));

    auto t0 = chrono::high_resolution_clock::now();

    size_t moveIdx = 0;
    int    turn     = 0;

    while (true) {
        turn++;
        MonteCarloTreeSearch mcts(res.board);
        int bestCol = mcts.bestMove();

        int aiCell = (bestCol >= 1 && bestCol <= g_board_cols) ? lowestFreeCell(res.board, bestCol) : 0;
        if (aiCell == 0) {
            for (int c = 1; c <= g_board_cols; c++) {
                aiCell = lowestFreeCell(res.board, c);
                if (aiCell != 0) break;
            }
        }
        // #region agent log
        {
            ostringstream d;
            d << "{\"game\":\"MCTS\",\"turn\":" << turn << ",\"bestCol\":" << bestCol << ",\"aiCell\":" << aiCell << "}";
            debugLog("main.cpp:runMcts_ai", "ai_move", d.str());
        }
        // #endregion
        if (aiCell != 0)
            res.board[aiCell] = 'O';

        uint8_t w = negaWinningFromBoard(res.board);
        if (w != 0) {
            res.winner = w;
            break;
        }

        if (moveIdx >= moves.size()) break;
        int col  = moves[moveIdx++];
        int cell = lowestFreeCell(res.board, col);
        if (cell == 0) {
            while (moveIdx < moves.size()) {
                col  = moves[moveIdx++];
                cell = lowestFreeCell(res.board, col);
                if (cell != 0) break;
            }
        }
        // #region agent log
        {
            ostringstream d;
            d << "{\"game\":\"MCTS\",\"turn\":" << turn << ",\"inputCol\":" << col << ",\"cell\":" << cell << "}";
            debugLog("main.cpp:runMcts_human", "human_move", d.str());
        }
        // #endregion
        if (cell != 0)
            res.board[cell] = 'X';

        w = negaWinningFromBoard(res.board);
        if (w != 0) {
            res.winner = w;
            break;
        }
    }

    // #region agent log
    {
        ostringstream d;
        d << "{\"game\":\"MCTS\",\"winner\":" << res.winner << "}";
        debugLog("main.cpp:runMcts", "game_end", d.str());
    }
    // #endregion
    auto t1          = chrono::high_resolution_clock::now();
    res.durationMs = chrono::duration<double, milli>(t1 - t0).count();
    return res;
}

static void printResult(const GameResult& r) {
    cout << "\n══════════════════════════════════════════════════════\n";
    cout << "  AI: " << r.aiName << "\n";
    cout << "══════════════════════════════════════════════════════\n";

    printBoard(r.board);

    cout << "\n  Rezultat:  ";
    if (r.winner == 1)
        cout << "X (Igrač) pobedio\n";
    else if (r.winner == 2)
        cout << "O (" << r.aiName << " AI) pobedio\n";
    else if (r.winner == 3)
        cout << "Nerešeno\n";
    else
        cout << "Igra nije završena (nema više poteza)\n";

    cout << "  Trajanje:  " << r.durationMs << " ms\n";
}

static void promptBoardDimensions() {
    cout << "Kvadratna tabla N x N. Kolone u input.txt su 1 … N.\n";
    cout << "Unesite N (" << kBoardSideMin << "–" << kBoardSideMax << "): ";

    int n = 0;
    for (;;) {
        if (!(cin >> n)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Neispravan unos. Unesite ceo broj " << kBoardSideMin << "–" << kBoardSideMax << ": ";
            continue;
        }
        if (boardSetSquareSize(n)) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Tabla: " << g_board_rows << " x " << g_board_cols << ".\n";
            return;
        }
        cout << "N mora biti izmedju " << kBoardSideMin << " i " << kBoardSideMax << ". Ponovo: ";
    }
}

int main() {
    promptBoardDimensions();

    const string inputPath = "input.txt";
    vector<int>  moves     = loadMoves(inputPath);

    cout << "Pokrecemo dve igre paralelno (NegaMax vs MCTS)...\n";
    cout << "Pobuda iz: " << inputPath << " (" << moves.size() << " poteza)\n";

    auto futureNega = async(launch::async, runNegamax, cref(moves));
    auto futureMcts = async(launch::async, runMcts, cref(moves));

    GameResult negaResult = futureNega.get();
    GameResult mctsResult = futureMcts.get();

    printResult(negaResult);
    printResult(mctsResult);

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
