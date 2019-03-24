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

/* works like a markov chain
* start with weights of w1 in every square for white pieces and b1 for black pieces
* start simulating from side_to_move
* for every legal move from a total of M attacks for a piece P, assign a weight of 1/M to all squares which P attacks
* do the same for the next side to move
* now normalize both weights to sum to 1
* as long as there is a positive weight remaining, distribute again in the same way
* some squares will become heavily influenced by 1 color after some time
* total score for the position is the sum of all white influence - sum of all black influence
* special treatment of kings - a king attack cannot be distributed to a square with `heavy' influence of the opposite side 
*/

double Features::evalStatic(const std::shared_ptr<Position> pos)
{
	double score[2][64];

	auto side = pos->side_to_move();
	
	if (pos->is_mate()) {
		return side == Color::WHITE ? -1 : 1;
	}

	for (Square s = SQ_A1; s <= SQ_H8; s++) {
		auto piece_color = pos->color_of_piece_on(s);
		if (piece_color == Color::WHITE) {
			score[0][s] = 1.0;
			score[1][s] = 0.0;
		}
		else if (piece_color == Color::BLACK) {
			score[0][s] = 0.0;
			score[1][s] = 1.0;
		}
		else {
			score[0][s] = 0.0;
			score[1][s] = 0.0;
		}
	}

	//Move mlist[256];
	//auto num_legal_moves = pos->all_legal_moves(mlist);
	//for (auto i = 0; i < num_legal_moves; i++) {
	//}

	int to_move = side == Color::WHITE ? 0 : 1;

	int rounds = 0;

	double sum[2];

	double prevSc = 0.0;

	while (true) {
		//printf("Round %d\n", rounds);
		//if (rounds == 100) {
		//	break;
		//}
		auto color = to_move == 0 ? Color::WHITE : Color::BLACK;
		// Set scores for a list of attack squares
		auto setScores = [&](Square sq, Bitboard attack_squares) {
			auto num_attacks = count_1s(attack_squares);
			//printf("%s\n", square_to_string(sq).c_str());
			//printf("Num attacks %d\n", num_attacks);
			while (attack_squares) {
				auto sqa = pop_1st_bit(&attack_squares);
				score[to_move][sqa] += (score[to_move][sq] / num_attacks);
			}
		};

		// Queen
		for (int i = 0; i < pos->queen_count(color); i++) {
			auto sq = pos->queen_list(color, i);
			auto sq_attacks_queen = pos->queen_attacks(sq);
			//printf("%lld\n", sq_attacks_queen);
			//auto num_attacks = count_1s(sq_attacks_queen);
			//printf("%d\n", num_attacks);
			setScores(sq, sq_attacks_queen);
		}

		// Rook
		for (int i = 0; i < pos->rook_count(color); i++) {
			auto sq = pos->rook_list(color, i);
			auto sq_attacks_rook = pos->rook_attacks(sq);
			setScores(sq, sq_attacks_rook);
		}

		// Bishop
		for (int i = 0; i < pos->bishop_count(color); i++) {
			auto sq = pos->bishop_list(color, i);
			auto sq_attacks_bishop = pos->bishop_attacks(sq);
			setScores(sq, sq_attacks_bishop);
		}

		// Knight
		for (int i = 0; i < pos->knight_count(color); i++) {
			auto sq = pos->knight_list(color, i);
			auto sq_attacks_knight = pos->knight_attacks(sq);
			setScores(sq, sq_attacks_knight);
		}

		// Pawn
		for (int i = 0; i < pos->pawn_count(color); i++) {
			auto sq = pos->pawn_list(color, i);
			auto sq_attacks_pawn = color == Color::WHITE ? pos->white_pawn_attacks(sq) : pos->black_pawn_attacks(sq);
			setScores(sq, sq_attacks_pawn);
		}

		// King
		auto sq = pos->king_square(color);
		auto sq_attacks_king = pos->king_attacks(sq);
		setScores(sq, sq_attacks_king);
	
		// normalize scores
		
		for (int j = 0; j < 64; j++) {
			auto ws = score[0][j];
			auto bs = score[1][j];

			score[0][j] = ws / (ws + bs + 0.00001);
			score[1][j] = bs / (ws + bs + 0.00001);
		}
		
		//// print scores
		//for (int i = 0; i < 2; i++) {
		//	for (Square s = SQ_A1; s <= SQ_H8; s++) {
		//		if (score[i][s] > 0.00001) {
		//			printf("[%s %.2lf]", square_to_string(s).c_str(), score[i][s]);
		//		}
		//	}
		//	printf("\n");
		//}

		to_move = 1 - to_move;
		if (to_move == (side == Color::WHITE ? 0 : 1)) {
			rounds++;
			for (int i = 0; i < 2; i++) {
				sum[i] = 0;
				for (int j = 0; j < 64; j++) {
					sum[i] += score[i][j];
				}
			}

			auto Sc = sum[0] - sum[1];
			if (std::fabs(Sc - prevSc) < 0.001) {
				break;
			}

			prevSc = Sc;
			//printf("Score %lf\n", prevSc);
		}
		//getchar();
	}

	//for (int i = 0; i < 2; i++) {
	//	sum[i] = 0;
	//	for (int j = 0; j < 64; j++) {
	//		sum[i] += score[i][j];
	//	}
	//}

	return (sum[0] - sum[1]) / 64;
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

void Features::setFeaturesFromPos(const std::shared_ptr<Position> pos)
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
	// TODO : make these king safety parameters
	// TODO : can castle does not work because after castling it favors the opposite side, so it actually prevents castling
	
	V[7] = 0; // int(pos->can_castle_kingside (Color::WHITE))  - int(pos->can_castle_kingside(Color::BLACK));
	V[8] = 0; // int(pos->can_castle_queenside(Color::WHITE)) - int(pos->can_castle_queenside(Color::BLACK));
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
