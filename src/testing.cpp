#include "testing.h"

#include "movegen.h"
#include "moveupd.h"

U64 perft(Position *pos, int depth, int original_depth) {
  Move move_list[256];
  int n_moves, i;
  U64 nodes = 0;
  n_moves = gen_legal_moves(pos, move_list);
  if (depth == 1) return (U64)n_moves;
  for (i = 0; i < n_moves; i++) {
    do_move(pos, move_list[i]);
    U64 cnt = perft(pos, depth - 1, original_depth);
    nodes += cnt;
    if (depth == original_depth) {
      U8 from = move_get_from(move_list[i]);
      U8 to = move_get_to(move_list[i]);
      printf("DEPTH(%d) %d %c%c%c%c: ", depth, move_get_flags(move_list[i]),
             'a' + (from & 7), '1' + (from >> 3), 'a' + (to & 7),
             '1' + (to >> 3));

      printf("%llu\n", cnt);
    }
    undo_move(pos, move_list[i]);
  }
  return nodes;
}
