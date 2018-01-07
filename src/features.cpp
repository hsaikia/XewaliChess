/* 
 * author: Himangshu Saikia, 2018
 * email : saikia@kth.se
*/
#include "features.h"

Features::Features()
{
	init_mersenne();
	init_direction_table();
	init_bitboards();
	Position::init_zobrist();
	Position::init_piece_square_tables();
	MovePicker::init_phase_table();
}

std::vector<double> Features::getFeatureVector() const
{
	return V;
}

void Features::setFeaturesFromPos(const std::shared_ptr<Position> pos)
{
	auto side = pos->side_to_move();

	V.clear();
	V.resize(numFeatures_);

	//V[0] = pos->queen_count(Color::WHITE) - pos->queen_count(Color::BLACK);
	//V[1] = pos->bishop_count(Color::WHITE) - pos->bishop_count(Color::BLACK);
	//V[2] = pos->rook_count(Color::WHITE) - pos->rook_count(Color::BLACK);
	//V[3] = pos->knight_count(Color::WHITE) - pos->knight_count(Color::BLACK);
	//V[4] = pos->pawn_count(Color::WHITE) - pos->pawn_count(Color::BLACK);
	V[0] = side == Color::WHITE ? 1 : -1;

	//V[5] = 0; // white attacks - black attacks

	
	for (Square s = SQ_A1; s <= SQ_H8; s++) {
		for (Color c = WHITE; c <= BLACK; c++) {
			// Queen
			for (int i = 0; i < pos->queen_count(c); i++) {
				auto sq = pos->queen_list(c, i);
				if (c == WHITE) {
					V[1] += pos->queen_attacks_square(sq, s);
				}
				else {
					V[1] -= pos->queen_attacks_square(sq, s);
				}
			}

			//Bishops
			for (int i = 0; i < pos->bishop_count(c); i++) {
				auto sq = pos->bishop_list(c, i);
				if (c == WHITE) {
					V[2] += pos->bishop_attacks_square(sq, s);
				}
				else {
					V[2] -= pos->bishop_attacks_square(sq, s);
				}
			}

			//Rooks
			for (int i = 0; i < pos->rook_count(c); i++) {
				auto sq = pos->rook_list(c, i);
				if (c == WHITE) {
					V[3] += pos->rook_attacks_square(sq, s);
				}
				else {
					V[3] -= pos->rook_attacks_square(sq, s);
				}
			}

			//Knights
			for (int i = 0; i < pos->knight_count(c); i++) {
				auto sq = pos->knight_list(c, i);
				if (c == WHITE) {
					V[4] += pos->knight_attacks_square(sq, s);
				}
				else {
					V[4] -= pos->knight_attacks_square(sq, s);
				}
			}

			//Pawns
			for (int i = 0; i < pos->pawn_count(c); i++) {
				auto sq = pos->pawn_list(c, i);
				if (c == WHITE) {
					V[5] += pos->white_pawn_attacks_square(sq, s);
				}
				else {
					V[5] -= pos->black_pawn_attacks_square(sq, s);
				}
			}

			//King
			auto sq = pos->king_square(c);
			if (c == WHITE) {
				V[6] += pos->king_attacks_square(sq, s);
			}
			else {
				V[6] -= pos->king_attacks_square(sq, s);
			}
			
		}
	}

	for (int i = 1; i < 7; i++) {
		V[i] /= 28;
	}

	V[7] = int(pos->can_castle_kingside(Color::WHITE)) - int(pos->can_castle_kingside(Color::BLACK));
	V[8] = int(pos->can_castle_queenside(Color::WHITE)) - int(pos->can_castle_queenside(Color::BLACK));
	V[9] = pos->is_check() ? -V[0] : 0;
}

void Features::printFeatureVector() const
{
	std::cout << "[";
	for (int i = 0; i < V.size(); i++) {
		std::cout << V[i] << " ";
	}
	std::cout << "]\n";
}

size_t Features::getNumFeatures()
{
	return numFeatures_;
}
