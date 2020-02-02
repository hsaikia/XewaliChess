#include "mcts_engine.h"
#include "features.h"
#include <climits>
#include <algorithm>

MctsEngine::MctsEngine()
{
	rng_.seed(1);
}

std::string MctsEngine::playMove(const int totalPlayouts, bool debug)
{
	isReady_ = false;
	int result;

	if (Engine::checkTermination(*pos_, result, false))
	{
		isReady_ = true;
		return "";
	}

	Move mlist[256];
	bool whiteToMove = (pos_->side_to_move() == Color::WHITE);

	//auto fen = pos_->to_fen(); // for debugging

	auto numLegalMoves = pos_->all_legal_moves(mlist);
	
	//std::uniform_int_distribution<size_t> distribution(0, num_legal_moves - 1);
	//auto randMoveIdx = distribution(rng_);
	//auto move = mlist[randMoveIdx];

	Chess::Move move;
	std::vector<Playouts> playouts(numLegalMoves);

	for (size_t i = 0; i < numLegalMoves; i++)
	{
		playouts[i].moveIdx = i;
		playouts[i].setScore(whiteToMove, 0);
	}

	const int depth = 100;

	for (size_t N = 0; N < totalPlayouts; N++) 
	{
		// sort scores
		std::sort(playouts.begin(), playouts.end(), Playouts::comparePlayouts);
		// pick the one with highest score
		size_t i = whiteToMove ? 0 : numLegalMoves - 1;
		size_t idx = playouts[i].moveIdx;
		//std::cout << move_to_string(move) << std::endl;

		auto res = performPlayout(pos_, mlist[idx], depth);
		playouts[i].addResult(res);
		for (size_t j = 0; j < numLegalMoves; j++)
		{
			playouts[j].setScore(whiteToMove, N + 1);
		}
	}

	// Playouts are done, now sort them by winRate
	std::sort(playouts.begin(), playouts.end(), Playouts::comparePlayoutsByWinRate);
	// best move
	move = whiteToMove ? mlist[playouts[0].moveIdx] : mlist[playouts[numLegalMoves - 1].moveIdx];

	if (debug)
	{
		for (size_t i = 0; i < numLegalMoves; i++)
		{
			auto m = mlist[playouts[i].moveIdx];
			auto sc = playouts[i].score;
			auto wr = playouts[i].winRate;
			auto games = playouts[i].games;
			auto w = playouts[i].whiteWins;
			auto b = playouts[i].blackWins;
			std::cout << "Move " << move_to_string(m) << " Score " << sc << " Win Rate " << wr << "(" << w << "/" << b << "/" << games << ")" << std::endl;
		}
	}

	// make the move on the current position
	UndoInfo u;
	pos_->do_move(move, u);
	isReady_ = true;

	return move_to_string(move);
}

int MctsEngine::performPlayout(const std::shared_ptr<Position> pos, const Chess::Move move, int depth)
{
	UndoInfo u;
	std::unique_ptr<Position> posTemp = std::make_unique<Position>(*pos);
	posTemp->reset_game_ply();
	posTemp->do_move(move, u);
	int result = 0;

	// Decide the outcome at a certain depth
	int numMoves = 2 * depth;
	while(numMoves --)
	{
		// check termination
		bool end = Engine::checkTermination(*posTemp, result, false);

		if (end)
		{
			//std::cout << "Game ended with result " << result << std::endl;
			return result;
		}

		Move mlist[256];
		auto numLegalMoves = posTemp->all_legal_moves(mlist);
		std::uniform_int_distribution<size_t> distribution(0, numLegalMoves - 1);
		auto randMoveIdx = distribution(rng_);
		auto randMove = mlist[randMoveIdx];

		UndoInfo ui;
		posTemp->do_move(randMove, ui);
	}

	auto res = Features::evalStaticMaterialOnly(*posTemp);

	//std::cout << "Game reached end of depth. Material " << res << std::endl;
	if (res > 0.0) 
	{
		return 1;
	}
	else if (res < 0.0)
	{
		return -1;
	}
	else 
	{
		return 0;
	}
	
	return 0;
}

Playouts::Playouts()
{
	games = 0;
	whiteWins = 0;
	blackWins = 0;
	winRate = 0.0;
}

void Playouts::addResult(int result)
{
	games++;
	if (result == 1)
	{
		whiteWins++;
	}
	else if (result == -1)
	{
		blackWins++;
	}
}

bool Playouts::comparePlayouts(const Playouts & a, const Playouts & b)
{
	return a.score > b.score;
}

bool Playouts::comparePlayoutsByWinRate(const Playouts & a, const Playouts & b)
{
	return a.winRate > b.winRate;
}

void Playouts::setScore(const bool rootPosWhiteToMove, const size_t totalGames)
{
	if (games == 0 || totalGames == 0)
	{
		if (rootPosWhiteToMove)
		{
			score = DBL_MAX;
		}
		else
		{
			score = -DBL_MAX;
		}
		return;
	}

	auto wd = static_cast<double>(whiteWins);
	auto bd = static_cast<double>(blackWins);
	auto ni = static_cast<double>(games);
	auto N = static_cast<double>(totalGames);

	double c = 1.5;

	if (rootPosWhiteToMove)
	{
		winRate = wd / ni;
		score = wd / ni + c * sqrt(log(N) / ni);
	}
	else
	{
		winRate = -bd / ni;
		score = -(bd / ni + c * sqrt(log(N) / ni));
	}
}
