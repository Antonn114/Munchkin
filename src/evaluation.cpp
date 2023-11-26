#include "evaluation.h"

#include "bittricks.h"
#include "movegen.h"

int check_winner(Position* pos) {
  Move move_list[256];
  int n_moves = gen_legal_moves(pos, move_list);
  if (n_moves == 0)
    if (pos->attacks_on_king[pos->side_2_move])
      return pos->side_2_move == mWhite ? EVAL_WINNER_BLACK : EVAL_WINNER_WHITE;
    else
      return EVAL_STALEMATE;
  else
    return EVAL_NULL;
}
// don't look at this

int eval(Position* pos) {
  int winner = check_winner(pos);
  if (winner != EVAL_NULL) {
    return winner * 1000;  // i don't KNOW!!!
  }
  U32 black_mat =
      popCount(pos->bb[mPawn] & pos->bb[mBlack]) +
      popCount((pos->bb[mBishop] | pos->bb[mKnight]) & pos->bb[mBlack]) * 3 +
      popCount(pos->bb[mRook] & pos->bb[mBlack]) * 5 +
      popCount(pos->bb[mQueen] & pos->bb[mBlack]) * 9;
  U32 white_mat =
      popCount(pos->bb[mPawn] & pos->bb[mWhite]) +
      popCount((pos->bb[mBishop] | pos->bb[mKnight]) & pos->bb[mWhite]) * 3 +
      popCount(pos->bb[mRook] & pos->bb[mWhite]) * 5 +
      popCount(pos->bb[mQueen] & pos->bb[mWhite]) * 9;
  return white_mat - black_mat;
}
// i suck
