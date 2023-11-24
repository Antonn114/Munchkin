#include <stdio.h>

#include "misc.h"
#include "munchkin.h"

U64 Perft(Position *pos, int depth) {
  Move move_list[256];
  int n_moves, i;
  U64 nodes = 0;
  if (depth == 0) return 1ULL;
  n_moves = gen_legal_moves(pos, move_list);
  for (i = 0; i < n_moves; i++) {
    do_move(pos, move_list[i]);
    U64 cnt = Perft(pos, depth - 1);
    nodes += cnt;
    undo_move(pos, move_list[i]);
  }
  return nodes;
}

int main(void) {
  munchkin_init();
  printf("Hello, World!\n");
  Position *pos = new Position;
  parse_FEN(pos, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 ");
  for (int i = 0; i < 3; i++) {
    printf("PERFT: %d %llu\n", i, Perft(pos, i));
  }
  getchar();

  printf("VRAM USED (KB): %d\n", getValueVRAM());
  printf("RAM USED (KB): %d\n", getValueRAM());
  return 0;
}
