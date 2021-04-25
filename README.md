# Xewali Chess

## Algorithm
- Xewali uses the minimax algorithm using alpha-beta pruning. 
- It orders moves using iterative deepening so that better lines are searched early on for the pruning step to be more effective. 
- It also searches for all subsequent return captures on a square if the search ends at a capture move. 
- [Piece tables](https://www.chessprogramming.org/Simplified_Evaluation_Function) to amplify importance of a piece at certain squares is implemented to attract pieces to good squares.
- Transposition tables are implemented to search at much deeper levels and not to re-evaluate positions already evaluated at an equal or higher depth.


## Evaluation
Xewali uses a static evaluation function which consists of static material difference, piece-table weighting and mobility. Extensive approaches are always being experimented upon.

## Credits
The move generation code is from the [Stockfish repository](https://github.com/daylen/stockfish-mac/tree/master/Chess)

## Test positions
```
ucinewgame
position startpos moves e2e4 e7e5 f1e2 a7a6 e2g4 b7b6 d1f3 c7c6 a2a4 d7d6 a1a3 f7f6 a3d3 h7h5 b1c3
go

ucinewgame
position startpos moves e2e4 e7e5 g1f3 d7d6 f1c4 b8a6 b1c3 c8g4
go

ucinewgame
position startpos
go

ucinewgame
position startpos moves e2e4 a7a6 d2d4 a6a5 g1f3 a5a4 f1c4 a4a3 b2b3 b7b6 b1a3
go

ucinewgame
position startpos moves e2e4 d7d5 e4d5 c7c6 d5c6 a7a6 c6b7 c8b7 b1c3
go

ucinewgame
position fen 5k2/7B/8/8/1n1n4/8/8/Q3K3 b - - 0 1
go

ucinewgame
position fen k7/5ppp/8/5PPP/8/8/8/K7 w - - 0 1 
go

ucinewgame
position fen 3r2k1/Qp4p1/7p/3p4/8/5PP1/4r2P/5qNK b - - 0 32 
go

ucinewgame
position fen r4b1r/ppp2pp1/3k1q2/7p/2P5/P2N3Q/2PN1PPP/4R1K1 b - - 3 21 
go

ucinewgame
position fen 8/3KP3/8/8/8/8/6k1/7q b - - 14 1
go 

```

- End Game - needs transposition table to solve
```
ucinewgame
position fen 8/k7/3p4/p2P1p2/P2P1P2/8/8/K7 w - - 0 1 
go
```

- Only move
```
ucinewgame
position fen 2kR1b1r/ppp2ppp/7n/4p3/2B1N3/4Bn2/PPP2P1P/2K3R1 b - - 0 16 
go
```