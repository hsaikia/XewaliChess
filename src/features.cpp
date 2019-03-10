/* 
 * author: Himangshu Saikia, 2018
 * email : saikia@kth.se
*/
#include "features.h"
#include "mersenne.h"
#include "movepick.h"

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

double Features::evalStatic() const
{
	double ret = 
		0.5 * V[0] // side to move - tempo
		+ 1.0 * (V[1] + V[2] + V[3] + V[4] + V[5]) // influence
		+ 0.2 * (V[7] + V[8]) // castling rights
		+ 0.1 * V[9] // is in check
		+ 10.0 * (V[10] * 9 + V[11] * 3 + V[12] * 5 + V[13] * 3 + V[14]) // material
		+ 5 * V[15]; //7th rank pawn

	if (V[16] != 0) {
		ret = V[16] * 300.0; // checkmate
	}

	return ret / 300.0;
}

void Features::setFeaturesFromPos(const std::shared_ptr<Position> pos)
{
	setFeaturesFromPos2(pos);
}

/*
void Features::setFeaturesFromPos(const std::shared_ptr<Position> pos) {

	V.clear();

	// Color white = 0. black = 1
	// Pieces : PAWN = 1, KNIGHT = 2, BISHOP = 3, ROOK = 4, QUEEN = 5, KING = 6
	// Squares : 64
	// 2 * 6 * 12 = 768
	// idx =  square * 12 + color * 6 + (piece - 1)

	// side to move = 1 bit
	// W can castle = 2 bit
	// B can castle = 2 bit
	// en passant available = 1 bit

	// total = 774

	V.resize(774, 0);

	for (Square S = SQ_A1; S <= SQ_H8; S++) {

		if (S == pos->ep_square()) {
			V[773] = pos->side_to_move() == Color::WHITE ? 1 : -1;
		}

		if (!pos->square_is_occupied(S)) {
			continue;
		}

		auto pt = pos->type_of_piece_on(S);
		auto c = pos->color_of_piece_on(S);

		V[12 * S + 6 * c + pt - 1] = 1;
	}

	V[768] = pos->side_to_move() == Color::WHITE ? 1 : -1;
	V[769] = pos->can_castle_kingside(Color::WHITE) ? 1 : 0;
	V[770] = pos->can_castle_queenside(Color::WHITE) ? 1 : 0;
	V[771] = pos->can_castle_kingside(Color::BLACK) ? 1 : 0;
	V[772] = pos->can_castle_queenside(Color::BLACK) ? 1 : 0;	
}
*/



// using only square infleunce
void Features::setFeaturesFromPos1(const std::shared_ptr<Position> pos) {
	
	auto side = pos->side_to_move();
	V.clear();
	V.resize(numFeatures_);
	
	//V[0] = side == Color::WHITE ? 1 : -1;

	//pos->piece_attacks_square

	for (Square s = SQ_A1; s <= SQ_H8; s++) {
		for (int i = 0; i < pos->queen_count(side); i++) {
			auto sq = pos->queen_list(side, i);
			if (side == WHITE) {
				V[0] += pos->queen_attacks_square(sq, s);
			}
			else {
				V[0] -= pos->queen_attacks_square(sq, s);
			}
		}
	}
}

void Features::setFeaturesFromPos2(const std::shared_ptr<Position> pos)
{
	auto side = pos->side_to_move();

	V.clear();
	V.resize(numFeatures_);

	V[0] = side == Color::WHITE ? 1 : -1;

	//V[5] = 0; // white attacks - black attacks

	//pos->
	
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

	//for (int i = 1; i < 7; i++) {
	//	V[i] /= 28;
	//}

	// TODO - make these king safety parameters
	// can castle does not work because after castling it favors the opposite side, so it actually prevents castling
	
	V[7] = 0; // int(pos->can_castle_kingside(Color::WHITE)) - int(pos->can_castle_kingside(Color::BLACK));
	V[8] = 0; //  int(pos->can_castle_queenside(Color::WHITE)) - int(pos->can_castle_queenside(Color::BLACK));
	V[9] = pos->is_check() ? -V[0] : 0;

	// piece counts
	V[10] = pos->queen_count(Color::WHITE) - pos->queen_count(Color::BLACK);
	V[11] = pos->bishop_count(Color::WHITE) - pos->bishop_count(Color::BLACK);
	V[12] = pos->rook_count(Color::WHITE) - pos->rook_count(Color::BLACK);
	V[13] = pos->knight_count(Color::WHITE) - pos->knight_count(Color::BLACK);
	V[14] = pos->pawn_count(Color::WHITE) - pos->pawn_count(Color::BLACK);

	//for (int i = 10; i < 15; i++) {
	//	V[i] /= 2;
	//}

	V[15] = pos->has_pawn_on_7th(Color::WHITE) - pos->has_pawn_on_7th(Color::BLACK);

	// Is MATE!
	V[16] = 0;
	if (pos->is_mate()) {
		if (side == WHITE) {
			V[16] = -1;
		}
		else {
			V[16] = 1;
		}
	}

	/*
	// Color white = 0. black = 1
	// Pieces : PAWN = 1, KNIGHT = 2, BISHOP = 3, ROOK = 4, QUEEN = 5, KING = 6
	
	int attack_and_defend[2][2][6][6]; // 144 features
	memset(attack_and_defend, 0, sizeof(attack_and_defend));

	for (Square fr = SQ_A1; fr <= SQ_H8; fr++) {

		if (!pos->square_is_occupied(fr)) {
			continue;
		}

		for (Square to = SQ_A1; to <= SQ_H8; to++) {
			if (fr == to) {
				continue;
			}

			if (!pos->square_is_occupied(to)) {
				continue;
			}

			if (!pos->piece_attacks_square(fr, to)) {
				continue;
			}
		
			auto pieceFr = pos->type_of_piece_on(fr);
			auto pieceTo = pos->type_of_piece_on(to);

			auto pieceFrColor = pos->color_of_piece_on(fr);
			auto pieceToColor = pos->color_of_piece_on(to);

			attack_and_defend[pieceFrColor][pieceToColor][pieceFr][pieceTo]++;
		}
	}
	*/
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
