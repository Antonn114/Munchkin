#include "testing.h"

#include <iostream>

#include "movegen.h"
#include "moveupd.h"

U64 perft(Position *pos, int depth, int original_depth) {
  Move move_list[256];
  int n_moves, i;
  U64 nodes = 0;
  n_moves = gen_legal_moves(pos, move_list);
  if (depth == 1) return n_moves;
  for (i = 0; i < n_moves; i++) {
    do_move(pos, move_list[i]);
    U64 cnt = perft(pos, depth - 1, original_depth);
    nodes += cnt;
    if (depth == original_depth) {
      std::cout << move_as_str(move_list[i]) << ": " << cnt << std::endl;
    }
    undo_move(pos, move_list[i]);
  }
  return nodes;
}
