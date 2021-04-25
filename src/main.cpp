/*
* author: Himangshu Saikia, 2018-2021
* email : himangshu.saikia.iitg@gmail.com
*/

#include "Xewali/ab_id_engine.h"
#include "Xewali/evaluation.h"
#include <ctime>
#include <sstream>
#include <iostream>

void tokenize(std::string line, std::vector<std::string>& tokens)
{
	tokens.clear();
	std::stringstream ss(line);
	std::string s;

	while (getline(ss, s, ' '))
	{
		tokens.push_back(s);
	}
}

int ucimain()
{
	// initializes the bitboards
	AbIterDeepEngine::init();

	// load the book moves
	Evaluation::Book book;
	Evaluation::load_games(book, "./uci_games.txt");

	Position pos;
	double currentEvaluation = 0.;
	std::string line;
	while (getline(std::cin, line))
	{
		std::vector<std::string> tokens;
		tokenize(line, tokens);

		if (tokens.size() == 0)
		{
			continue;
		}

		if (tokens[0] == "uci")
		{
			std::cout << "id name Xewali 1.0" << std::endl;
			std::cout << "id author Himangshu Saikia" << std::endl;
			std::cout << "uciok" << std::endl;
		}
		else if (tokens[0] == "ucinewgame")
		{
			// nothing to init
		}
		else if (tokens[0] == "isready")
		{
			std::cout << "readyok" << std::endl;
		}
		else if (tokens[0] == "position")
		{
			std::string fen = "";
			std::vector<std::string> moves;
			bool reading_fen = true;

			if (tokens[1] == "startpos")
			{
				fen = StartPosition;
			}
			else if (tokens[1] == "fen")
			{
				// do nothing
			}

			for (int i = 2; i < tokens.size(); i++)
			{
				if (tokens[i] == "moves")
				{
					reading_fen = false;
					continue;
				}
				if (reading_fen)
				{
					fen += tokens[i];
					fen += " ";
				}
				else
				{
					moves.push_back(tokens[i]);
				}
			}
			AbIterDeepEngine::set_position(pos, fen, moves);
		}
		else if (tokens[0] == "go")
		{
			std::cout << "Position\n";
			pos.print();
			std::cout << "info Thinking..." << std::endl;
			std::cout << "bestmove " << AbIterDeepEngine::play_move(pos, currentEvaluation, book) << std::endl;
		}
		else if (tokens[0] == "quit")
		{
			break;
		}
		else if (tokens[0] == "eval")
		{
			std::cout << currentEvaluation << std::endl;
		}
		else
		{
			//nothing to do
		}
	}
	return 0;
}

int main() {
	ucimain();
	return 0;
}
