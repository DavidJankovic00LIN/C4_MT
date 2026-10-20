# spec_v3 — checklist specifikacije

## Trenutna implementacija (4 igrača)

| Figura | Uloga | Fajlovi |
|--------|-------|---------|
| `X` | NegaMax | `negamax.*` |
| `O` | MCTS | `mcts.*` |
| `+` | Greedy | `greedy.*` |
| `@` | Potezi iz `input.txt` | `input_player.*` |

- Tabla **N×N**, N sa konzole **7–15**
- Indeksiranje **NegaMax**: `index = col + N*row`, kolone 1…N, indeks 0 neiskorišćen
- Gravitacija na dno; pobeda = 4 iste u nizu
- Redosled: `X → O → + → @`
- AI hot path koristi lokalnu proveru `winsFromCell` (full `findWinner` ostaje za `main`)
- NegaMax dubina `MAX_DEPTH = 6`, bez alpha-beta
- MCTS je stablo (`kMctsIterations = 800`), ne playout po koloni

Build: `make` → `./spec_v3_runner`

---

## A) Šta ima u `Desktop/VP_C4/Spec` (klasična specifikacija)

| # | Element | Status u Spec |
|---|---------|----------------|
| 1 | Tabla **6×7** (hardkod) | Da |
| 2 | Indeksiranje **NegaMax** | Da |
| 3 | **2 figure**: `X`, `O` | Da |
| 4 | `Board()` — iscrtavanje | Da |
| 5 | `GetValue(column)` | Da |
| 6 | `winning()` | Da |
| 7 | `clean()` | Da |
| 8 | `PlayPosition` / `input.txt` | Da |
| 9 | `AIManager` + `NegaMax` | Da |
| 10 | AI igra prvi | Da |
| 11 | Provokacija | Da |
| 12 | Jedan AI | Da |

---

## B) Rešeno u `spec_v3`

| # | Odluka | Status |
|---|--------|--------|
| 1 | N×N, N 7–15 | Da (`boardSetSize`) |
| 2 | NegaMax indeksiranje | Da |
| 3 | 4 figure X O + @ | Da |
| 4 | `drawBoard` | Da |
| 5 | `applyMove` / `findWinner` / `isBoardFull` | Da |
| 6 | NegaMax (X) | Da |
| 7 | MCTS (O), stablo (UCT / ekspanzija / playout / backprop) | Da |
| 8 | Greedy (+) | Da |
| 9 | Input player (@) | Da |
| 10 | Game loop u `main` | Da |

---

## C) Kasnije (nije u ovom koraku)

| # | Stavka |
|---|--------|
| 1 | ~~Sliding-window / lokalna `winsFromCell`~~ (urađeno) |
| 2 | Alpha-beta / EVA / PlayOut / provocation |
| 3 | Paralelne partije / poređenje kao u `mt/` |
| 4 | VP / SystemC / BRAM |
