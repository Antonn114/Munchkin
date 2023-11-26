#include "search.h"

#include <assert.h>

#include "evaluation.h"
#include "limits.h"
#include "movegen.h"
#include "moveupd.h"
int search(Position* pos, int depth, int alpha, int beta) {
  if (depth == 0) {
    return eval(pos) * (pos->side_2_move == mWhite ? 1 : -1);
  }
  Move move_list[256];
  U8 n_moves = gen_legal_moves(pos, move_list);
  int best_score = -INT_MAX;
  for (int i = 0; i < n_moves; i++) {
    do_move(pos, move_list[i]);
    int score = -search(pos, depth - 1, -beta, -alpha);
    best_score = std::max(best_score, score);
    undo_move(pos, move_list[i]);
    // shut up
    alpha = std::max(alpha, best_score);
    if (alpha >= beta) break;
  }
  return best_score;
}

Move get_best_move(Position* pos, int depth) {
  Move move_list[256];
  U8 n_moves = gen_legal_moves(pos, move_list);
  assert(n_moves);
  int best_score = -INT_MAX;
  Move best_move = move_list[0];
  for (int i = 1; i < n_moves; i++) {
    do_move(pos, move_list[i]);
    int score = -search(pos, depth - 1, -INT_MAX, INT_MAX);
    undo_move(pos, move_list[i]);
    if (score > best_score) {
      best_score = score;
      best_move = move_list[i];
    }
  }
  return best_move;
}
