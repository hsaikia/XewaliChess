# Converts a pgn file (with multiple games)
# to a txt file with all games converted to uci notation
# for the xewali executable to be able to read

import chess.pgn
import io

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