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

double Features::evalStaticMaterialOnly(const Position& pos)
{
	/*
	* MATERIAL
	* Calculate attack reachability of every piece on an empty board

	* Queen :	
	Corners and edges (8x8, 28) : 21 = 588
	Corners and edges (6x6, 20) : 23 = 460
	Corners and edges (4x4, 12) : 25 = 300
	Corners and edges (2x2, 4)  : 27 = 108

	WEIGHTED AVERAGE OF QUEEN : 22.75

	* Rook :	
	All Squares on board (64) : 14

	WEIGHTED AVERAGE OF ROOK : 14

	* Bishop:	
	Corners and edges (8x8, 28) : 7 = 196
	Corners and edges (6x6, 20) : 9 = 180
	Corners and edges (4x4, 12) : 11 = 132
	Corners and edges (2x2, 4)  : 13 = 52

	WEIGHTED AVERAGE OF BISHOP : 8.75

	* Knight:	
	Corner Squares (4)								: 2 = 8
	Edge squares adjacent to corner square (8)		: 3 = 24
	Other Edge squares (16)							: 4 = 64
	Corner Squares (6x6, 4)							: 4 = 16
	Edge squares (6x6, 16)							: 6 = 96
	Center Squares (4x4, 16)						: 8 = 128

	WEIGHTED AVERAGE OF KNIGHT : 5.25

	* Pawn :	
	Rank (2-7, not rook files, 36)  : 2 = 72
	Rank (2-7,     rook files, 12)  : 1 attack squares = 12
	8th rank (1, any file, 8)       : average of rook/queen/knight/bishop moves at given square
	: Assuming this is a queen = 8 * 21 = 168

	WEIGHTED AVERAGE OF PAWN : 3.9375 (considering queen promotion)
	WEIGHTED AVERAGE OF PAWN : 1.3125 (not considering queen promotion)

	* King :    
	Corner (4)					: 3 = 12
	Edges except corners (24)	: 5 = 120
	All other squares (36)		: 8 = 288
	(not considering castling as it's not an attack move)

	WEIGHTED AVERAGE OF KING : 6.5625

	*/

	std::vector< std::vector<double> > material(2, std::vector<double>(64, 0.0));
	// calculate material
	for (Square s = SQ_A1; s <= SQ_H8; s++) {
		auto piece = pos.piece_on(s);
		switch (piece) {
		case Piece::WQ: material[0][s] = 22.75;		material[1][s] = 0.0; break;
		case Piece::WR: material[0][s] = 14.0;		material[1][s] = 0.0; break;
		case Piece::WB: material[0][s] = 8.75;		material[1][s] = 0.0; break;
		case Piece::WN: material[0][s] = 5.25;		material[1][s] = 0.0; break;
		case Piece::WP: material[0][s] = 1.3125;	material[1][s] = 0.0; break;
		case Piece::WK: material[0][s] = 6.5625;	material[1][s] = 0.0; break;
		case Piece::BQ: material[1][s] = 22.75;		material[0][s] = 0.0; break;
		case Piece::BR: material[1][s] = 14.0;		material[0][s] = 0.0; break;
		case Piece::BB: material[1][s] = 8.75;		material[0][s] = 0.0; break;
		case Piece::BN: material[1][s] = 5.25;		material[0][s] = 0.0; break;
		case Piece::BP: material[1][s] = 1.3125;	material[0][s] = 0.0; break;
		case Piece::BK: material[1][s] = 6.5625;	material[0][s] = 0.0; break;
		case Piece::NO_PIECE: material[0][s] = 0.0;	material[1][s] = 0.0; break;
		default: break;
		}
	}

	double sum_material[2];

	for (int i = 0; i < 2; i++) {
		sum_material[i] = 0.0;
		for (int j = 0; j < 64; j++) {
			sum_material[i] += material[i][j];
		}
	}
	return sum_material[0] - sum_material[1];
}

double Features::evalStaticAttackDefenseOnly(Position & pos)
{
	double attacks[2];
	attacks[0] = 0.0;
	attacks[1] = 0.0;

	for (Color color = WHITE; color <= BLACK; color++)
	{
		// Queen
		for (int i = 0; i < pos.queen_count(color); i++) {
			auto sq = pos.queen_list(color, i);
			auto sq_attacks = pos.queen_attacks(sq);
			auto num_attacks = count_1s(sq_attacks);
			attacks[color] += num_attacks;
		}

		// Rook
		for (int i = 0; i < pos.rook_count(color); i++) {
			auto sq = pos.rook_list(color, i);
			auto sq_attacks = pos.rook_attacks(sq);
			auto num_attacks = count_1s(sq_attacks);
			attacks[color] += num_attacks;
		}

		// Bishop
		for (int i = 0; i < pos.bishop_count(color); i++) {
			auto sq = pos.bishop_list(color, i);
			auto sq_attacks = pos.bishop_attacks(sq);
			auto num_attacks = count_1s(sq_attacks);
			attacks[color] += num_attacks;
		}

		// Knight
		for (int i = 0; i < pos.knight_count(color); i++) {
			auto sq = pos.knight_list(color, i);
			auto sq_attacks = pos.knight_attacks(sq);
			auto num_attacks = count_1s(sq_attacks);
			attacks[color] += num_attacks;
		}

		// Pawn
		for (int i = 0; i < pos.pawn_count(color); i++) {
			auto sq = pos.pawn_list(color, i);
			auto sq_attacks = color == Color::WHITE ? pos.white_pawn_attacks(sq) : pos.black_pawn_attacks(sq);
			auto num_attacks = count_1s(sq_attacks);
			attacks[color] += num_attacks;
		}

		// King
		auto sq = pos.king_square(color);
		auto sq_attacks = pos.king_attacks(sq);
		auto num_attacks = count_1s(sq_attacks);
		attacks[color] += num_attacks;
	}
	return (attacks[0] - attacks[1]);
}

double Features::evalStatic(Position& pos)
{
	const double lambda = 0.1;
	auto net_eval = lambda * (evalStaticAttackDefenseOnly(pos))  +  (1 - lambda) * (evalStaticMaterialOnly(pos));

	//always return between [1, 1]
	if (net_eval < -1) {
		return -1;
	}
	else if (net_eval > 1) {
		return 1;
	}
	
	return net_eval;	
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

		if (S == pos.ep_square()) {
			V[773] = pos.side_to_move() == Color::WHITE ? 1 : -1;
		}

		if (!pos.square_is_occupied(S)) {
			continue;
		}

		auto pt = pos.type_of_piece_on(S);
		auto c = pos.color_of_piece_on(S);

		V[12 * S + 6 * c + pt - 1] = 1;
	}

	V[768] = pos.side_to_move() == Color::WHITE ? 1 : -1;
	V[769] = pos.can_castle_kingside(Color::WHITE) ? 1 : 0;
	V[770] = pos.can_castle_queenside(Color::WHITE) ? 1 : 0;
	V[771] = pos.can_castle_kingside(Color::BLACK) ? 1 : 0;
	V[772] = pos.can_castle_queenside(Color::BLACK) ? 1 : 0;	
}
*/

void Features::setFeaturesFromPos(Position& pos)
{
	auto side = pos.side_to_move();

	V.clear();
	V.resize(numFeatures_);

	V[0] = side == Color::WHITE ? 1 : -1;

	//V[5] = 0; // white attacks - black attacks

	//pos.
	
	for (Square s = SQ_A1; s <= SQ_H8; s++) {
		for (Color c = WHITE; c <= BLACK; c++) {
			// Queen
			for (int i = 0; i < pos.queen_count(c); i++) {
				auto sq = pos.queen_list(c, i);
				if (c == WHITE) {
					V[1] += pos.queen_attacks_square(sq, s);
				}
				else {
					V[1] -= pos.queen_attacks_square(sq, s);
				}
			}

			//Bishops
			for (int i = 0; i < pos.bishop_count(c); i++) {
				auto sq = pos.bishop_list(c, i);
				if (c == WHITE) {
					V[2] += pos.bishop_attacks_square(sq, s);
				}
				else {
					V[2] -= pos.bishop_attacks_square(sq, s);
				}
			}

			//Rooks
			for (int i = 0; i < pos.rook_count(c); i++) {
				auto sq = pos.rook_list(c, i);
				if (c == WHITE) {
					V[3] += pos.rook_attacks_square(sq, s);
				}
				else {
					V[3] -= pos.rook_attacks_square(sq, s);
				}
			}

			//Knights
			for (int i = 0; i < pos.knight_count(c); i++) {
				auto sq = pos.knight_list(c, i);
				if (c == WHITE) {
					V[4] += pos.knight_attacks_square(sq, s);
				}
				else {
					V[4] -= pos.knight_attacks_square(sq, s);
				}
			}

			//Pawns
			for (int i = 0; i < pos.pawn_count(c); i++) {
				auto sq = pos.pawn_list(c, i);
				if (c == WHITE) {
					V[5] += pos.white_pawn_attacks_square(sq, s);
				}
				else {
					V[5] -= pos.black_pawn_attacks_square(sq, s);
				}
			}

			//King
			auto sq = pos.king_square(c);
			if (c == WHITE) {
				V[6] += pos.king_attacks_square(sq, s);
			}
			else {
				V[6] -= pos.king_attacks_square(sq, s);
			}	
		}
	}

	//for (int i = 1; i < 7; i++) {
	//	V[i] /= 28;
	//}
	// TODO : make these king safety parameters
	// TODO : can castle does not work because after castling it favors the opposite side, so it actually prevents castling
	
	V[7] = 0; // int(pos.can_castle_kingside (Color::WHITE))  - int(pos.can_castle_kingside(Color::BLACK));
	V[8] = 0; // int(pos.can_castle_queenside(Color::WHITE)) - int(pos.can_castle_queenside(Color::BLACK));
	V[9] = pos.is_check() ? -V[0] : 0;

	// piece counts
	V[10] = pos.queen_count(Color::WHITE) - pos.queen_count(Color::BLACK);
	V[11] = pos.bishop_count(Color::WHITE) - pos.bishop_count(Color::BLACK);
	V[12] = pos.rook_count(Color::WHITE) - pos.rook_count(Color::BLACK);
	V[13] = pos.knight_count(Color::WHITE) - pos.knight_count(Color::BLACK);
	V[14] = pos.pawn_count(Color::WHITE) - pos.pawn_count(Color::BLACK);

	//for (int i = 10; i < 15; i++) {
	//	V[i] /= 2;
	//}

	V[15] = pos.has_pawn_on_7th(Color::WHITE) - pos.has_pawn_on_7th(Color::BLACK);

	// Is MATE!
	V[16] = 0;
	if (pos.is_mate()) {
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

		if (!pos.square_is_occupied(fr)) {
			continue;
		}

		for (Square to = SQ_A1; to <= SQ_H8; to++) {
			if (fr == to) {
				continue;
			}

			if (!pos.square_is_occupied(to)) {
				continue;
			}

			if (!pos.piece_attacks_square(fr, to)) {
				continue;
			}
		
			auto pieceFr = pos.type_of_piece_on(fr);
			auto pieceTo = pos.type_of_piece_on(to);

			auto pieceFrColor = pos.color_of_piece_on(fr);
			auto pieceToColor = pos.color_of_piece_on(to);

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
