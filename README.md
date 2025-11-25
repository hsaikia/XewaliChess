# Xewali Chess

The inner workings of XewaliChess is explained in details in [this](https://www.youtube.com/watch?v=E7FGXCbwImI) video!

## Algorithm
- Xewali uses the [minimax](https://www.chessprogramming.org/Minimax) algorithm using [alpha-beta pruning](https://www.chessprogramming.org/Alpha-Beta). 
- It orders moves using [internal iterative deepening](https://www.chessprogramming.org/Internal_Iterative_Deepening) so that better lines are searched early on for the pruning step to be more effective. 
- It also searches for all subsequent return captures on a square if the search ends at a capture move. 
- [Piece tables](https://www.chessprogramming.org/Simplified_Evaluation_Function) to amplify importance of a piece at certain squares is implemented to attract pieces to good squares.
- [Transposition tables](https://www.chessprogramming.org/Transposition_Table) are implemented to search at much deeper levels and not to re-evaluate positions already evaluated at an equal or higher depth.

## Play on Lichess 

[Xewali BOT](https://lichess.org/@/xewali)

## Evaluation
Xewali uses a static evaluation function which consists of static material difference, piece-table weighting and mobility. Extensive approaches are always being experimented upon.

## To use book

Place the file `uci_games.txt` along with the executable after building.

## Example Game (Xewali vs Xewali)

```
[Event "Computer chess game"]
[Site "DESKTOP-A62U312"]
[Date "2021.04.25"]
[Round "?"]
[White "XewaliEngine"]
[Black "XewaliEngine"]
[Result "1-0"]
[BlackElo "2200"]
[ECO "C60"]
[Opening "Spanish"]
[Time "21:44:43"]
[Variation "Cozio, Paulsen Variation"]
[WhiteElo "2200"]
[Termination "normal"]
[PlyCount "125"]
[WhiteType "program"]
[BlackType "program"]

1. e4 g6 2. Nc3 Nc6 3. Nf3 e5 4. Bb5 Nge7 5. Bxc6 Nxc6 6. d4 exd4 7. Nxd4
Bg7 8. Nxc6 Bxc3+ 9. bxc3 dxc6 10. Qxd8+ Kxd8 11. Rb1 Rg8 12. Bg5+ Ke8 13.
c4 f5 14. e5 Kf7 15. O-O Re8 16. Rfe1 Kg8 17. c5 Rb8 18. Rbd1 b6 19. cxb6
Rxb6 20. Rd8 Rxd8 21. Bxd8 Kf7 22. Bxc7 Rb7 23. Bd6 Rb2 24. Rc1 Rxa2 25.
Kf1 Be6 26. Ke2 f4 27. Kf3 Ra4 28. g3 Bd5+ 29. Ke2 Be4 30. gxf4 Ra2 31. Ke3
Bxc2 32. Bb4 Ke6 33. Kd4 Kf5 34. Bd2 Ra4+ 35. Kc5 Be4 36. h4 Bd5 37. Be3 a6
38. Kd6 Ra2 39. Rc5 Ra4 40. e6 Kf6 41. f5 Re4 42. e7 Kxf5 43. Kd7 h6 44.
Bxh6 Kg4 45. Ra5 Bf7 46. Bg5 Kf3 47. Rxa6 Rd4+ 48. Kc7 Be8 49. Ra8 Rd7+ 50.
Kb6 Rxe7 51. Bxe7 Bd7 52. Bc5 Bf5 53. Re8 Kg4 54. Kxc6 Kxh4 55. Kd5 Kg5 56.
Bd6 Bb1 57. f3 Ba2+ 58. Ke5 Kh6 59. Kf6 Kh7 60. Be5 Bd5 61. Kg5 Bg8 62. f4
Bd5 63. Rh8# 1-0

```

## Credits
The move generation code is from the [Stockfish repository](https://github.com/daylen/stockfish-mac/tree/master/Chess)


## Test positions

- Starting Position
```
ucinewgame
position startpos
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

- Tricky move from game on lichess (15...Bb7 is a blunder!)
```
ucinewgame
position fen r1b2r1k/ppp3pp/3b1qn1/5p2/3N4/1Q3B2/PPP2PPP/R1BR2K1 w - - 6 15
go
```

- Other examples

```
ucinewgame
position startpos moves e2e4 e7e5 f1e2 a7a6 e2g4 b7b6 d1f3 c7c6 a2a4 d7d6 a1a3 f7f6 a3d3 h7h5 b1c3
go

ucinewgame
position startpos moves e2e4 e7e5 g1f3 d7d6 f1c4 b8a6 b1c3 c8g4
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
