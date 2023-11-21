#ifndef __MUNCHKIN_POSITION_H__
#define __MUNCHKIN_POSITION_H__

#include "defs.h"
#include <string>
#include "bitboard.h"

#define MOVEFLAG_QUIET  0
#define MOVEFLAG_PAWN_DPUSH   1
#define MOVEFLAG_CASTLE_KING  2
#define MOVEFLAG_CASTLE_QUEEN 3
#define MOVEFLAG_CAPTURE  4
#define MOVEFLAG_CAPTURE_EP   5
#define MOVEFLAG_PROMOTE_KNIGHT   6
#define MOVEFLAG_PROMOTE_BISHOP   7
#define MOVEFLAG_PROMOTE_ROOK     8
#define MOVEFLAG_PROMOTE_QUEEN    9
#define MOVEFLAG_PROMOTE_KNIGHT_CAPTURE   10
#define MOVEFLAG_PROMOTE_BISHOP_CAPTURE   11
#define MOVEFLAG_PROMOTE_ROOK_CAPTURE     12
#define MOVEFLAG_PROMOTE_QUEEN_CAPTURE    13

#define CASTLING_RIGHTS_BLACK_KING 1
#define CASTLING_RIGHTS_BLACK_QUEEN 2
#define CASTLING_RIGHTS_WHITE_KING 4
#define CASTLING_RIGHTS_WHITE_QUEEN 8
#define CASTLING_RIGHTS_ALL 15

typedef unsigned int Move;


U8 move_get_flags(Move m);
U8 move_get_to(Move m);
U8 move_get_from(Move m);

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



class Position{
private:
  U64 bitboard[8];
  U8 mailbox[64];
  U8 side_2_move;

  U8 castling_rights[6000];
  U8 en_passant_sq[6000];
  U8 halfmove_clock[6000];
  U32 ply;

  U64 king_danger_squares;
  U64 attacks_on_king;
  U64 capture_mask;
  U64 push_mask;
  U64 pinned_mask;
  U64 pinner_mask;

  U64 opponent_slider_to_king(int Esq, int Ksq);

  int king_square[2];
  void init_king_prerequisites();
  void gen_rook_moves(int sq);
  void gen_bishop_moves(int sq);
  void gen_knight_moves(int sq);
  void gen_pawn_moves(int sq);
  void gen_king_moves(int sq);
  void gen_pinned_pieces_moves();

  U8 captured_history_list[64];
  int captured_ptr = 0;
public:
  Move move_list[256];

  int move_ptr;

  bool is_slider(int sq);
  int gen_legal_moves(Move ml[]);
  void do_move(Move m);
  void undo_move(Move m);

  void parse_FEN(const std::string& fen);
  void display_mailbox();
  void debug_bitboard(int c);
};

#endif // !__MUNCHKIN_POSITION_H__
