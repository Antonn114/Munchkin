#ifndef __MUNCHKIN_POSITION_H__
#define __MUNCHKIN_POSITION_H__

#include "defs.h"
#include "bitboard.h"
#include "piece.h"

typedef unsigned int Move;

class Position{
private:
  enum m_Bitboard_squares{
    mWhite,
    mBlack,
    mPawn,
    mKnight,
    mBishop,
    mRook,
    mQueen,
    mKing
  };

  U64 bitboard[8];
  U8 mailbox[64];
  m_Bitboard_squares side_2_move;
  U8 castling_rights;
  enumSquare en_passant_sq;
  U32 halfmove_clock;

  U64 king_danger_squares;
  U64 attacks_on_king;
  U64 capture_mask;
  U64 push_mask;
  U64 pinned_mask;

  U64 opponent_slider_to_king(int Esq, int Ksq);

  int king_square[2];
  Move move_list[256];
  int move_ptr;
  void init_king_prerequisites();
  void gen_rook_moves(int sq);
  void gen_bishop_moves(int sq);
  void gen_knight_moves(int sq);
  void gen_pawn_moves(int sq);
  void gen_king_moves(int sq);
public:
  bool is_slider(int sq);
  void gen_legal_moves();
};

#endif // !__MUNCHKIN_POSITION_H__
