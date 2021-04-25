import chess.pgn
import io

# class Attacker():
#     def __init__(self, color, influence, sq):
#         #print(color, " ", influence, " ", sq)
#         self.color = color
#         self.influence = influence
#         self.sq = sq

# def evaluate(board):
#     attacks_on = OrderedDict()
#     for x in chess.SQUARES:
#         sq_from = chess.SQUARE_NAMES[x]
#         piece = board.piece_at(x)
#         board.
#         if piece == None:
#             continue
#         else:
#             color = chess.COLOR_NAMES[piece.color]
#             atks = board.attacks(x)
#             sqs_to = [chess.SQUARE_NAMES[sq] for sq in atks]
#             for sq_to in sqs_to :
#                 attacker = Attacker(color, len(sqs_to), sq_from)
#                 if sq_to in attacks_on :
#                     attacks_on[sq_to].append(attacker)
#                 else:
#                     attacks_on[sq_to] = [attacker]

#     for k, v in attacks_on.items():
#         print(f'\nSquare {k} is attacked by')
#         for atk in v :
#             print(f'{atk.color} Piece at {atk.sq} with influence {atk.influence}')

def convert_games(pgn_string):
    pgn = io.StringIO(pgn_string)
    game = chess.pgn.read_game(pgn)

    moves = game.mainline_moves()
    uci_str = ""
    for move in moves:
        uci_str = uci_str + move.uci() + " "
    return uci_str + "\n"


def main():
    file = open("games.pgn", "r")
    moves_seq = [line.strip() for line in file.readlines() if '[' not in line]

    pgns = []

    #board = chess.Board()

    moves = ""

    for move_line in moves_seq:
        if '1. ' in move_line and ' 2. ' in move_line:
            pgns.append(moves)
            moves = " " + move_line
        else:
            moves = moves + " " + move_line

    print(f'Total games {len(pgns)}')

    file_out = open("uci_games.txt", "w")

    for pgn in pgns[1:]:
        uci_game = convert_games(pgn)
        file_out.write(uci_game)

if __name__ == "__main__":
    main()