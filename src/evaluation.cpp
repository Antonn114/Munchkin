#include "evaluation.h"

#include "bittricks.h"
#include "movegen.h"
#include "position.h"

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
  Move move_list[256];
  U8 foo = pos->side_2_move;
  pos->side_2_move = mWhite;
  int white_moves = gen_legal_moves(pos, move_list);
  pos->side_2_move = mBlack;
  int black_moves = gen_legal_moves(pos, move_list);
  pos->side_2_move = foo;

  int winner = check_winner(pos);
  if (winner != EVAL_NULL) {
    return winner * 1000000;  // i don't KNOW!!!
  }
  int tot_pst_score = 0;
  for (int i = 0; i < 8; i++)
    tot_pst_score +=
        pos->pst_score[i] * ((i & 7) == mPawn && is_opening(pos) ? 2 : 1);
  for (int i = 8; i < 16; i++)
    tot_pst_score -=
        pos->pst_score[i] * ((i & 7) == mPawn && is_opening(pos) ? 2 : 1);
  int early_queen_penalty =
      (pos->bb[mQueen] & pos->bb[mWhite] && pos->bb[mQueen] & pos->bb[mBlack])
          ? is_opening(pos) *
                ((bitScanForward(pos->bb[mQueen] & pos->bb[mWhite]) >> 3) * 20 -
                 (7 -
                  (bitScanForward(pos->bb[mQueen] & pos->bb[mBlack]) >> 3)) *
                     20)
          : 0;
  U32 black_mat =
      popCount(pos->bb[mPawn] & pos->bb[mBlack]) * EVAL_MAT_PAWN +
      popCount(pos->bb[mKnight] & pos->bb[mBlack]) * EVAL_MAT_KNIGHT +
      popCount(pos->bb[mBishop] & pos->bb[mBlack]) * EVAL_MAT_BISHOP +
      popCount(pos->bb[mRook] & pos->bb[mBlack]) * EVAL_MAT_ROOK +
      popCount(pos->bb[mQueen] & pos->bb[mBlack]) * EVAL_MAT_QUEEN;
  U32 white_mat =
      popCount(pos->bb[mPawn] & pos->bb[mWhite]) * EVAL_MAT_PAWN +
      popCount(pos->bb[mKnight] & pos->bb[mWhite]) * EVAL_MAT_KNIGHT +
      popCount(pos->bb[mBishop] & pos->bb[mWhite]) * EVAL_MAT_BISHOP +
      popCount(pos->bb[mRook] & pos->bb[mWhite]) * EVAL_MAT_ROOK +
      popCount(pos->bb[mQueen] & pos->bb[mWhite]) * EVAL_MAT_QUEEN;
  return white_mat - black_mat +
         ((white_moves - black_moves) - 4) * EVAL_MOBILITY + tot_pst_score -
         early_queen_penalty;
}
// i suck
