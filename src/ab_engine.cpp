#include "ab_engine.h"
#include "features.h"

AlphaBetaEngine::AlphaBetaEngine()
{
}

std::string AlphaBetaEngine::playMove(const int depth, bool debug)
{
	isReady_ = false;
	int result;

	if (Engine::checkTermination(*pos_, result, false))
	{
		isReady_ = true;
		return "";
	}

	Move mlist[256];
	size_t bestmove = 0;
	auto numLegalMoves = pos_->all_legal_moves(mlist);
	bool whiteToMove = (pos_->side_to_move() == Color::WHITE);
	double bestScore = whiteToMove ? -DBL_MAX : DBL_MAX;

	for (size_t i = 0; i < numLegalMoves; i++)
	{
		//std::cout << " Move " << move_to_string(mlist[i]) << "\n";
		UndoInfo u;
		std::unique_ptr<Position> posTemp = std::make_unique<Position>(*pos_);
		posTemp->do_move(mlist[i], u);
		double eval = minimax(*posTemp, depth, -DBL_MAX, DBL_MAX);
		if (whiteToMove)
		{
			if (eval > bestScore)
			{
				bestmove = i;
				bestScore = eval;
			}
		}
		else 
		{
			if (eval < bestScore)
			{
				bestmove = i;
				bestScore = eval;
			}
		}
		std::cout << "Move " << move_to_string(mlist[i]) << " Evaluation : " << eval << std::endl;
	}

	std::cout << "Best Move " << move_to_string(mlist[bestmove]) << " Evaluation : " << bestScore << std::endl;

	// make the move on the current position
	UndoInfo u;
	pos_->do_move(mlist[bestmove], u);
	isReady_ = true;

	return move_to_string(mlist[bestmove]);
}

double AlphaBetaEngine::minimax(Position & pos, const int depth, double alpha, double beta)
{
	if (depth == 0)
	{
		return 0.4 * Features::evalStaticAttackDefenseOnly(pos) + 0.6 * Features::evalStaticMaterialOnly(pos);
	}

	int result;
	if (Engine::checkTermination(pos, result, false))
	{
		isReady_ = true;
		return result * 999999;
	}

	//std::cout << "A " << alpha << " B " << beta << " depth " << depth << std::endl;

	Move mlist[256];
	auto numLegalMoves = pos.all_legal_moves(mlist);
	bool whiteToMove = (pos.side_to_move() == Color::WHITE);

	if (whiteToMove)
	{
		double maxEval = -DBL_MAX;
		for (size_t i = 0; i < numLegalMoves; i++)
		{
			UndoInfo u;
			pos.do_move(mlist[i], u);
			//std::cout << "Calling minimax on move " << move_to_string(mlist[i]) << std::endl;
			double eval = minimax(pos, depth - 1, alpha, beta);
			pos.undo_move(mlist[i], u);
			maxEval = max(maxEval, eval);
			alpha = max(alpha, eval);
			if (beta <= alpha)
			{
				break;
			}
		}
		return maxEval;
	}
	else 
	{
		double minEval = DBL_MAX;
		for (size_t i = 0; i < numLegalMoves; i++)
		{
			UndoInfo u;
			pos.do_move(mlist[i], u);
			//std::cout << "Calling minimax on move " << move_to_string(mlist[i]) << std::endl;
			double eval = minimax(pos, depth - 1, alpha, beta);
			pos.undo_move(mlist[i], u);
			minEval = min(minEval, eval);
			beta = min(beta, eval);
			if (beta <= alpha)
			{
				break;
			}
		}
		return minEval;
	}
}

//void AlphaBetaEngine::sortMoves(Move mlist[], size_t numLegalMoves)
//{
//	for(size_t i = 0; i < numLegalMoves; i++)
//	{
//		for (size_t j = 1; j < numLegalMoves; j++)
//		{
//			if( mlist[j])
//		}
//	}
//}

