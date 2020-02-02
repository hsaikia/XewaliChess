# XewaliChess

## Algorithm

Currently Xewali uses the minimax algorithm using alpha-beta pruning for best performance. A few other algorithms are also implemented (variants of MCTS), but they are not quite as strong.

## Evaluation

Xewali uses a static evaluation function which is a combination of material value and reachability of pieces on the board.

## Credits

The move generation code is from the Stockfish repository : https://github.com/daylen/stockfish-mac/tree/master/Chess
