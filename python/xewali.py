from collections import OrderedDict
import chess

class Attacker():
    def __init__(self, color, influence, sq):
        #print(color, " ", influence, " ", sq)
        self.color = color
        self.influence = influence
        self.sq = sq

def evaluate(board):
    attacks_on = OrderedDict()
    for x in chess.SQUARES:
        sq_from = chess.SQUARE_NAMES[x]
        piece = board.piece_at(x)
        board.
        if piece == None:
            continue
        else:
            color = chess.COLOR_NAMES[piece.color]
            atks = board.attacks(x)
            sqs_to = [chess.SQUARE_NAMES[sq] for sq in atks]
            for sq_to in sqs_to :
                attacker = Attacker(color, len(sqs_to), sq_from)
                if sq_to in attacks_on :
                    attacks_on[sq_to].append(attacker)
                else:
                    attacks_on[sq_to] = [attacker]

    for k, v in attacks_on.items():
        print(f'\nSquare {k} is attacked by')
        for atk in v :
            print(f'{atk.color} Piece at {atk.sq} with influence {atk.influence}')



def main():
    board = chess.Board()
    board.push_san("e4")
    board.push_san("e5")
    board.push_san("Nf3")
    board.push_san("Nc6")
    board.push_san("Bc4")
    board.push_san("Qf6")
    print(board)
    print(board.turn)
    evaluate(board)

main()