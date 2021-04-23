# Xewali Chess

## Algorithm
Currently Xewali uses the minimax algorithm using alpha-beta pruning. It orders moves using iterative deepening so that better lines are searched early on for the pruning step to be more effective. It also searches for all subsequent return captures on a square if the search ends at a capture move.

## Evaluation
Xewali uses a static evaluation function which consists of material only. Extensive approaches are always being experimented upon.

## Credits
The move generation code is from the Stockfish repository : https://github.com/daylen/stockfish-mac/tree/master/Chess

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
position fen r1bqr1k1/pp3ppp/2nb1n2/8/P2p4/1P1P1PPP/8/RNBQKBNR w KQ - 1 12
go

ucinewgame
position fen 5k2/7B/8/8/1n1n4/8/8/Q3K3 b - - 0 1
go

ucinewgame
position fen k7/8/8/7p/6P1/8/8/K7 w - - 0 1
go

ucinewgame
position startpos moves e2e4 e7e5
go

ucinewgame
position fen r4b1r/ppp2pp1/3k1q2/7p/2P5/P2N3Q/2PN1PPP/4R1K1 b - - 3 21 
go

```


