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
	Evaluation::load_games(book, "./engines/uci_games.txt");

	Position pos;
	double currentEvaluation = 0.;
	std::string line;
	while (getline(std::cin, line))
	{
		//std::cout << "Command received [" << line << "]\n";
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
			// std::cout << "echo Book is loaded with " << book.size() << " positions\n";
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
			double time_to_move = 1.0;
			if (tokens.size() == 9 && tokens[1] == "wtime" && tokens[3] == "btime" && tokens[5] == "winc" && tokens[7] == "binc")
			{
				const auto wtime = std::atoi(tokens[2].c_str());
				const auto btime = std::atoi(tokens[4].c_str());
				const auto winc = std::atoi(tokens[6].c_str());
				const auto binc = std::atoi(tokens[8].c_str());

				time_to_move = pos.side_to_move() == Color::WHITE ? (wtime + winc) / 60000.0 : (btime + binc) / 60000.0;
			}
			time_to_move = time_to_move > 5.0 ? 5.0 : time_to_move;
			//pos.print();
			std::cout << "info Thinking..." << std::endl;
			std::cout << "bestmove " << AbIterDeepEngine::play_move(pos, currentEvaluation, book, time_to_move) << std::endl;
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
