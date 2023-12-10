#include "uci.h"

#include <iostream>
#include <sstream>
#include <string>

#include "movegen.h"
#include "moveupd.h"
#include "position.h"
#include "search.h"
#include "testing.h"
void command_handler(Position* pos) {
  std::string line;

  std::getline(std::cin, line);
  std::istringstream iss(line);
  std::string command_prefix;
  iss >> command_prefix;
  if (command_prefix == "uci") {
    std::cout << "id name Munchkin\nid author Anton\nuciok" << std::endl;
  } else if (command_prefix == "isready") {
    std::cout << "readyok" << std::endl;
  } else if (command_prefix == "position") {
    std::string fen_command;
    iss >> fen_command;
    if (fen_command == "fen") {
      std::string fen_str;
      std::string foo;
      while (iss >> foo) {
        if (foo == "moves") break;
        fen_str += foo;
        fen_str += ' ';
      }
      parse_FEN(pos, fen_str);
    } else if (fen_command == "startpos") {
      parse_FEN(pos,
                "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
      iss >> fen_command;
    }
    std::string move;
    while (iss >> move) {
      if (move.length() < 4) continue;
      Move m = parse_move(pos, move.c_str());
      do_move(pos, m);
    }
  } else if (command_prefix == "go") {
    std::string foo;
    bool output_best_move = true;
    while (iss >> foo) {
      if (foo == "perft") {
        output_best_move = false;
        iss >> foo;
        int res = perft(pos, stoi(foo), stoi(foo));
        std::cout << "\n" << res << std::endl;
        break;
      }
    }
    if (output_best_move) {
      Move m;
      if (is_opening(pos)) {
        m = get_best_move(pos, 6);
      } else if (is_middle(pos)) {
        m = get_best_move(pos, 6);
      } else {
        m = get_best_move(pos, 16);
      }
      std::cout << "bestmove " << move_as_str(m) << std::endl;
    }
  } else if (command_prefix == "quit") {
    exit(0);
  }
}
