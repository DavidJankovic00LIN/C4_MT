# spec_v3 — konstante i varijable

Spisak po fajlu gde nastaju.  
Obuhvata imenovane konstante, globale, članske promenljive i bitne magične brojeve.  
Sitne lokalne petlje (`i`, privremeni `score`…) nisu navedene.

**Indeksiranje table (NegaMax)**

```
index = col1 + g_n * row0
col1  ∈ [1 … g_n]     (kolona, 1-based)
row0  ∈ [0 … g_n−1]   (red; 0 = vrh)
index 0               neiskorišćen
```

---

## Sadržaj

1. [board.hpp](#1-boardhpp)
2. [board.cpp](#2-boardcpp)
3. [main.cpp](#3-maincpp)
4. [mcts.cpp](#4-mctscpp--mctshpp)
5. [negamax.cpp](#5-negamaxcpp--negamaxhpp)
6. [greedy.cpp](#6-greedycpp--greedyhpp)
7. [input_player](#7-input_playerhpp--input_playercpp)
8. [draw_board](#8-draw_boardcpp--draw_boardhpp)
9. [Makefile](#9-makefile)
10. [AI parametri (ugađanje)](#10-ai-parametri-za-ugađanje)
11. [Arhitektura](#11-arhitektura)
12. [Opis funkcija](#12-opis-funkcija)
13. [Uloga fajlova](#13-uloga-fajlova)

---

## 1. `board.hpp`

### 1.1 Veličina table i bafer

| Ime                    | Vrsta              | Vrednost / tip              | Značenje                                      |
|------------------------|--------------------|-----------------------------|-----------------------------------------------|
| `BOARD_SIDE_MAX`       | `#define`          | `20` (override moguć)       | Max strana table u baferu pri kompilaciji     |
| `kBoardSideMin`        | `constexpr int`    | `7`                         | Najmanji dozvoljeni N sa konzole              |
| `kBoardSideMax`        | `constexpr int`    | `15`                        | Najveći dozvoljeni N sa konzole               |
| `BOARD_STORAGE_SIZE`   | `#define`          | `BOARD_SIDE_MAX² + 1`       | Veličina `char` bafera (+1 zbog indeksa 0)    |
| `kConnectLen`          | `constexpr int`    | `4`                         | Dužina niza za pobedu (connect-4)             |
| `g_n`                  | `extern int`       | (def. u `board.cpp`)        | Aktivna veličina table N×N u runtime-u        |

> `BOARD_SIDE_MAX` mora biti ≥ `kBoardSideMax`. Override: `-DBOARD_SIDE_MAX=…`

### 1.2 Figure

| Ime            | Vrsta             | Vrednost | Značenje                    |
|----------------|-------------------|----------|-----------------------------|
| `EMPTY`        | `constexpr char`  | `' '`    | Prazno polje                |
| `PIECE_X`      | `constexpr char`  | `'X'`    | Igrač X — NegaMax           |
| `PIECE_O`      | `constexpr char`  | `'O'`    | Igrač O — MCTS              |
| `PIECE_PLUS`   | `constexpr char`  | `'+'`    | Igrač + — Greedy            |
| `PIECE_AT`     | `constexpr char`  | `'@'`    | Igrač @ — `input.txt`       |

### 1.3 Pomoćne konstante u funkcijama

| Ime     | Gde                       | Tip / vrednost                                      | Značenje                          |
|---------|---------------------------|-----------------------------------------------------|-----------------------------------|
| `kDirs` | `winsFromCell`            | `int[4][2]` = `(1,0) (0,1) (1,1) (1,-1)`            | Pravci: H, V, dve dijagonale      |
| `kAll`  | `findBlockingColumn`      | `{ PIECE_X, PIECE_O, PIECE_PLUS, PIECE_AT }`        | Sve figure za proveru bloka       |

### 1.4 Česti parametri funkcija

| Ime                      | Tip                         | Značenje                                              |
|--------------------------|-----------------------------|-------------------------------------------------------|
| `board`                  | `char[BOARD_STORAGE_SIZE]`  | Stanje table                                          |
| `col1`                   | `int`                       | Kolona 1-based (`1 … g_n`)                            |
| `row0`                   | `int`                       | Red 0-based od vrha (`0 … g_n−1`)                     |
| `cell`                   | `int`                       | Ravni indeks; `0` = nevažeće / nema poteza            |
| `piece` / `me` / `opp`   | `char`                      | Figura koja igra / „ja“ / protivnik                   |
| `n` (`boardSetSize`)     | `int`                       | Predloženi N; validan u `[kBoardSideMin, kBoardSideMax]` |

---

## 2. `board.cpp`

| Ime    | Vrsta      | Vrednost   | Značenje                                                         |
|--------|------------|------------|------------------------------------------------------------------|
| `g_n`  | global int | početno `7`| Definicija aktivne veličine; menja je `boardSetSize(n)`          |

---

## 3. `main.cpp`

| Ime             | Vrsta              | Vrednost / tip              | Značenje                                      |
|-----------------|--------------------|-----------------------------|-----------------------------------------------|
| `board`         | lokalni niz        | `char[BOARD_STORAGE_SIZE]`  | Glavna tabla partije                          |
| `turn`          | lokalni `char`     | start = `PIECE_X`           | Trenutni igrač (`X → O → + → @`)              |
| `col`           | lokalni `int`      | 1-based                     | Izabrana kolona; `0` = nema poteza / greška   |
| `winner`        | lokalni `char`     | iz `findWinner`             | Pobednik; `EMPTY` = nema                      |
| `n`             | lokalni `int`      | unos sa stdin               | N u `promptBoardSize`                         |
| `inputPlayer`   | objekat            | `InputPlayer("input.txt")`  | Igrač `@`                                     |
| `"input.txt"`   | string literal     | putanja                     | Izvor kolona za `@`                           |

---

## 4. `mcts.cpp` / `mcts.hpp`

### 4.1 Konstante

| Ime                 | Vrsta              | Vrednost | Značenje                                              |
|---------------------|--------------------|----------|-------------------------------------------------------|
| `kMctsIterations`   | `constexpr int`    | `800`    | Broj iteracija stabla po potezu (ukupno, ne po koloni) |
| `kUctC`             | `constexpr double` | `1.41`   | Eksploracioni član UCT-a                              |
| `kPieces`           | `const char[4]`    | X O + @  | Redosled nagrada u čvoru (`reward[0..3]`)             |

### 4.2 Čvor `Node`

| Ime            | Tip                    | Značenje                                              |
|----------------|------------------------|-------------------------------------------------------|
| `col`          | `int`                  | Kolona poteza koji je doveo do čvora; koren je `0`    |
| `playedBy`     | `char`                 | Figura koja je odigrala `col`; koren je `EMPTY`       |
| `toMove`       | `char`                 | Igrač koji bira sledeći potez u ovom čvoru            |
| `winner`       | `char`                 | Pobednik ako je `terminal`; `EMPTY` = nerešeno        |
| `terminal`     | `bool`                 | Pobeda ili puna tabla; nema playout-a                 |
| `visits`       | `int`                  | Broj prolazaka kroz čvor                              |
| `reward`       | `double[4]`            | Zbir nagrada po figuri (`+1` pobeda, `-1` poraz)      |
| `untried`      | `int[BOARD_SIDE_MAX]`  | Legalne kolone koje još nisu proširene                |
| `untriedCount` | `int`                  | Broj neprobanih kolona                                |
| `children`     | `vector<Node*>`        | Prošireni potomci                                     |
| `parent`       | `Node*`                | Roditelj; koren je `nullptr`                          |

Koren je stanje pre poteza `me`. Stablo se briše na izlasku iz `chooseColumnMcts`.

### 4.3 Parametri i lokalne (bitne)

| Ime                  | Vrsta        | Značenje                                              |
|----------------------|--------------|-------------------------------------------------------|
| `me`                 | param `char` | Figura za koju MCTS bira (obično `O`)                 |
| `toMove`             | `char`       | Čiji je potez u playout-u                             |
| `work` / `sim`       | lokalni niz  | Kopija table za spuštanje kroz stablo / playout       |
| `actor`              | `char`       | Igrač čiju nagradu UCT maksimizuje (`parent->toMove`) |

### 4.4 Nagrada playout-a (po figuri, duž puta)

| Vrednost | Značenje                         |
|----------|----------------------------------|
| `+1`     | Ta figura je pobednik            |
| `-1`     | Pobedio je neko drugi            |
| `0`      | Nerešeno / puna tabla / prekid   |

---

## 5. `negamax.cpp` / `negamax.hpp`

### 5.1 Konstante

| Ime          | Vrsta             | Vrednost | Značenje                                       |
|--------------|-------------------|----------|------------------------------------------------|
| `MAX_DEPTH`  | `constexpr int`   | `6`      | Maks. dubina `search` (iza toga skor `0`)      |

### 5.2 Parametri `search` / root

| Ime                    | Vrsta        | Značenje                                              |
|------------------------|--------------|-------------------------------------------------------|
| `me`                   | param `char` | Figura koja se maksimizuje (obično `X`)               |
| `toMove`               | param `char` | Igrač na potezu u čvoru                               |
| `depth`                | param `int`  | Trenutna dubina (root kreće od `1`)                   |
| `lastCell`             | param `int`  | Poslednja odigrana ćelija za `winsFromCell`; `0`=nema |
| `order` / `orderCount` | lokalni      | Redosled kolona od centra ka ivicama                  |
| `center`               | lokalni int  | `(g_n + 1) / 2` — središnja kolona                    |
| `maximizing`           | lokalni bool | `true` ako `toMove == me`                             |
| `best`                 | lokalni int  | Najbolji skor u čvoru                                 |
| `bestCol`              | lokalni int  | Izabrana kolona na root-u                             |
| `bestScore`            | lokalni int  | Skor izabrane root kolone                             |

### 5.3 Terminalni skor

| Vrednost | Značenje                              |
|----------|---------------------------------------|
| `+1`     | Pobeda za `me`                        |
| `-1`     | Poraz za `me`                         |
| `0`      | Nerešeno, cutoff dubine, ili puna tabla |

---

## 6. `greedy.cpp` / `greedy.hpp`

### 6.1 Magični brojevi (heuristika)

| Ime / vrednost       | Značenje                                      |
|----------------------|-----------------------------------------------|
| `10`                 | Baza center-bonusa: `10 − \|col − center\|`   |
| `2`                  | Težina pretnji: `+ 2 * countThreats(...)`     |
| `-1000000`           | Početni `bestScore` (sentinel „nema još“)     |

### 6.2 Varijable

| Ime         | Vrsta        | Značenje                                              |
|-------------|--------------|-------------------------------------------------------|
| `me`        | param `char` | Figura Greedy igrača (obično `+`)                     |
| `threats`   | lokalni int  | Broj kolona gde bi `me` odmah pobedio                 |
| `center`    | lokalni int  | `(g_n + 1) / 2`                                       |
| `score`     | lokalni int  | Heuristika poteza                                     |
| `bestCol`   | lokalni int  | Najbolja kolona                                       |
| `bestScore` | lokalni int  | Najbolji heuristički skor                             |

---

## 7. `input_player.hpp` / `input_player.cpp`

### 7.1 Članovi klase `InputPlayer`

| Ime        | Tip                 | Početno   | Značenje                                |
|------------|---------------------|-----------|-----------------------------------------|
| `moves_`   | `std::vector<int>`  | prazan    | Kolone 1-based iz fajla                 |
| `index_`   | `size_t`            | `0`       | Sledeći indeks u `moves_`               |
| `loaded_`  | `bool`              | `false`   | Da li je fajl uspešno učitan            |

### 7.2 Ostalo

| Ime                 | Vrsta           | Značenje                                              |
|---------------------|-----------------|-------------------------------------------------------|
| `path`              | ctor param      | Putanja fajla (npr. `"input.txt"`)                    |
| `m`                 | lokalni int     | Jedna učitana kolona iz fajla                         |
| `legal` / `count`   | lokalni         | Fallback: nasumična legalna kolona kad lista nestane  |

---

## 8. `draw_board.cpp` / `draw_board.hpp`

| Ime        | Vrsta           | Vrednost / tip     | Značenje                                      |
|------------|-----------------|--------------------|-----------------------------------------------|
| `board`    | param           | `const char*`      | Tabla za ispis                                |
| `lineLen`  | lokalni int     | `g_n * 8 + 1`      | Širina separator-linije u konzoli             |
| `8`        | magični int     | —                  | Širina jedne kolone u ASCII crtežu            |

---

## 9. `Makefile`

| Ime          | Vrednost                              | Značenje                    |
|--------------|---------------------------------------|-----------------------------|
| `CXX`        | `g++`                                 | Kompajler                   |
| `CXXFLAGS`   | `-std=c++17 -Wall -Wextra -O2`        | Flegovi kompilacije         |
| `TARGET`     | `spec_v3_runner`                      | Izlazni binarni fajl        |
| `SRC`        | lista `.cpp`                          | Izvornici                   |
| `OBJ`        | lista `.o`                            | Objektni fajlovi            |

---

## 10. AI parametri (za ugađanje)

| Parametar              | Fajl           | Podrazumevano | Efekat                         |
|------------------------|----------------|---------------|--------------------------------|
| `kMctsIterations`      | `mcts.cpp`     | `800`         | Broj iteracija MCTS stabla     |
| `kUctC`                | `mcts.cpp`     | `1.41`        | UCT eksploracija               |
| `MAX_DEPTH`            | `negamax.cpp`  | `6`           | Jačina / cena NegaMax          |
| `kConnectLen`          | `board.hpp`    | `4`           | Pravilo pobede                 |
| `kBoardSideMin`        | `board.hpp`    | `7`           | Donja granica N                |
| `kBoardSideMax`        | `board.hpp`    | `15`          | Gornja granica N               |
| `g_n`                  | `board.cpp`    | runtime       | Aktivna veličina table         |
| Greedy `10` / `2`      | `greedy.cpp`   | `10` / `2`    | Center-bonus / težina pretnji  |

---

## 11. Arhitektura

`spec_v3` je konzolna Connect-4 varijanta na kvadratnoj tabli **N×N** (N ∈ [7, 15]), sa gravitacijom po kolonama i pobedom na **4** iste figure u nizu.

### 11.1 Igrači i strategije

| Figura | Modul            | Strategija                                      |
|--------|------------------|-------------------------------------------------|
| `X`    | `negamax`        | Pretraga unapred (NegaMax, dubina `MAX_DEPTH` = 6, bez alpha-beta) |
| `O`    | `mcts`           | Stablo: UCT, ekspanzija jednog poteza, playout, backprop (`kMctsIterations`) |
| `+`    | `greedy`         | Heuristika: centar + pretnje (+ win/block)      |
| `@`    | `input_player`   | Sekvenca kolona iz `input.txt` (+ random fallback)|

Redosled poteza: `X → O → + → @` (`nextPiece`).

### 11.2 Tok programa

1. Unos N (`promptBoardSize` → `boardSetSize`).
2. Inicijalizacija table (`clearBoard`) i učitavanje `input.txt`.
3. Petlja: izbor kolone po trenutnom igraču → `applyMove` → `drawBoard`.
4. Terminacija: `findWinner` (pobeda) ili `isBoardFull` (nerešeno).

Stanje table je ravni `char` bafer (`BOARD_STORAGE_SIZE`); aktivna dimenzija je global `g_n`.

### 11.3 Struktura modula

```
main
 ├── board         — reprezentacija, gravitacija, pobeda, win/block helperi
 ├── draw_board    — ASCII prikaz
 ├── negamax       — AI za X
 ├── mcts          — AI za O
 ├── greedy        — AI za +
 └── input_player  — AI / scripted potezi za @
```

AI hot-path za pobedu koristi lokalnu proveru `winsFromCell` (poslednja ćelija).  
Full-board `findWinner` ostaje u glavnoj petlji (`main`).

---

## 12. Opis funkcija

### 12.1 `board.hpp` / `board.cpp`

| Funkcija              | Uloga                                                                 |
|-----------------------|-----------------------------------------------------------------------|
| `boardSetSize`        | Postavlja `g_n` ako je N u `[kBoardSideMin, kBoardSideMax]`.          |
| `boardByteSize`       | Vraća broj bajtova aktivne table: `g_n² + 1`.                         |
| `cellIndex`           | Mapira `(col1, row0)` → linearni indeks (NegaMax layout).             |
| `lowestFreeCell`      | Najniža prazna ćelija u koloni (gravitacija); `0` ako je kolona puna. |
| `isLegalMove`         | Da li je vrh kolone prazan (potez dozvoljen).                         |
| `applyMove`           | Ubaci figuru u kolonu; `false` ako potez nije legalan.                |
| `applyMoveCell`       | Isto kao `applyMove`, ali vraća indeks ćelije gde je figura pala.     |
| `cellToColRow`        | Inverzna mapa: indeks ćelije → `(col1, row0)`.                        |
| `winsFromCell`        | Provera pobede oko zadate ćelije (4 pravca, O(`kConnectLen`)).        |
| `clearBoard`          | Popuni bafer sa `EMPTY`.                                              |
| `findWinner`          | Full-board scan svih pravaca; vraća figuru pobednika ili `EMPTY`.     |
| `isBoardFull`         | `true` ako nijedna kolona nema slobodan vrh.                          |
| `nextPiece`           | Ciklični sled igrača: X → O → + → @ → X.                              |
| `findWinningColumn`   | Kolona kojom `me` odmah pobedi; `0` ako nema.                         |
| `findBlockingColumn`  | Kolona kojom neki protivnik odmah pobedi (za blok); `0` ako nema.     |

### 12.2 `draw_board`

| Funkcija     | Uloga                                                          |
|--------------|----------------------------------------------------------------|
| `drawBoard`  | Ispis table u konzoli (zaglavlje kolona + ćelije + separator). |

### 12.3 `main.cpp`

| Funkcija           | Uloga                                                              |
|--------------------|--------------------------------------------------------------------|
| `pieceName`        | Mapira figuru na čitljiv label (npr. `"X (NegaMax)"`).             |
| `promptBoardSize`  | Interaktivni unos N dok `boardSetSize` ne uspe.                    |
| `main`             | Orchestracija: setup, petlja poteza, detekcija kraja partije.      |

### 12.4 `negamax` (figura X)

| Funkcija                | Uloga                                                                  |
|-------------------------|------------------------------------------------------------------------|
| `fillCenterOrder`       | Generiše redosled kolona od centra ka ivicama (move ordering).         |
| `evaluateTerminal`      | Skor terminalnog stanja: `+1` / `−1` / `0` u odnosu na `me`.           |
| `search`                | Rekurzivna NegaMax pretraga do `MAX_DEPTH` (6); terminal preko `lastCell`. Skor i dalje `+1` / `−1` / `0`, bez alpha-beta. |
| `chooseColumnNegamax`   | Win/block prvo; zatim root pretraga i izbor kolone sa najboljim skorom.|

### 12.5 `mcts` (figura O)

| Funkcija               | Uloga                                                                                          |
|------------------------|------------------------------------------------------------------------------------------------|
| `fillUntried`          | Upisuje legalne kolone koje čvor još nije proširio.                                            |
| `randomLegalColumn`    | Uniforman izbor među legalnim kolonama (za playout).                                           |
| `playout`              | Nasumična simulacija do kraja; vraća figuru pobednika ili `EMPTY`.                             |
| `uct` / `selectChild`  | Igrač na potezu bira dete sa najvećim UCT svoje nagrade; nepohođeno dete ima prioritet.        |
| `expand`               | Dodaje tačno jedan neprobani potez. Pobeda ili puna tabla označava čvor kao `terminal`.        |
| `backPropagate`        | `visits++` duž puta; pobeda `+1` toj figuri, ostalima `-1`, nerešeno `0`.                      |
| `bestRootMove`         | Dete korena sa najviše poseta; izjednačenje rešava veća prosečna nagrada `me`.                 |
| `chooseColumnMcts`     | Win/block prvo; zatim `kMctsIterations` iteracija stabla. Stablo se briše na izlasku.          |

### 12.6 `greedy` (figura +)

| Funkcija               | Uloga                                                                    |
|------------------------|--------------------------------------------------------------------------|
| `countThreats`         | Broj kolona u kojima bi `me` odmah pobedio (probe-drop + `winsFromCell`).|
| `chooseColumnGreedy`   | Win/block prvo; inače max heuristike: centar-bonus + `2 ×` pretnje.      |

### 12.7 `input_player` (figura @)

| Funkcija / metod              | Uloga                                                              |
|-------------------------------|--------------------------------------------------------------------|
| `InputPlayer` (ctor)          | Učitava listu kolona iz fajla u `moves_`.                          |
| `loaded`                      | Da li je fajl uspešno učitan.                                      |
| `hasMoves`                    | Da li je ostalo nepročitanih poteza u listi.                       |
| `chooseColumn`                | Sledeća legalna kolona sa liste; ako lista nestane — random legal. |

---

## 13. Uloga fajlova

| Fajl / modul       | Odgovornost                                              |
|--------------------|----------------------------------------------------------|
| `board.*`          | Model table, gravitacija, pobeda, taktički helperi       |
| `draw_board.*`     | Prezentacija stanja u konzoli                            |
| `main.cpp`         | Ulazna tačka i game-loop                                 |
| `negamax.*`        | Strategija za `X`                                        |
| `mcts.*`           | Strategija za `O`                                        |
| `greedy.*`         | Strategija za `+`                                        |
| `input_player.*`   | Scripted / fajl-vođeni potezi za `@`                     |
| `VARIABLES.md`     | Dokumentacija konstanti, arhitekture i API-ja            |
