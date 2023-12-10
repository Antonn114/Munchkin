#ifndef __MUNCHKIN_POSITION_H__
#define __MUNCHKIN_POSITION_H__

#include <string>

#include "bitboard.h"
#include "defs.h"

#define WHITE_PAWN_CH 'P'
#define WHITE_KNIGHT_CH 'N'
#define WHITE_BISHOP_CH 'B'
#define WHITE_ROOK_CH 'R'
#define WHITE_QUEEN_CH 'Q'
#define WHITE_KING_CH 'K'

#define BLACK_PAWN_CH 'p'
#define BLACK_KNIGHT_CH 'n'
#define BLACK_BISHOP_CH 'b'
#define BLACK_ROOK_CH 'r'
#define BLACK_QUEEN_CH 'q'
#define BLACK_KING_CH 'k'

#define CASTLING_RIGHTS_BLACK_KING 1
#define CASTLING_RIGHTS_BLACK_QUEEN 2
#define CASTLING_RIGHTS_WHITE_KING 4
#define CASTLING_RIGHTS_WHITE_QUEEN 8
#define CASTLING_RIGHTS_ALL 15

enum enumPiece {
  mWhite,
  mBlack,
  mPawn,
  mKnight,
  mBishop,
  mRook,
  mQueen,
  mKing
};

U8 board_to_bb(U8 c);
U8 bb_to_board(U8 c, U8 p);
struct Position {
  U64 bb[8];
  U8 board[64];
  U8 king_square[2];
  U8 side_2_move;

  U8 castling_rights[64];
  U8 en_passant_sq[64];
  U8 halfmove_clock[64];
  U8 fullmove;

  U64 pinned_mask;
  U64 capture_mask;
  U64 push_mask;
  U64 king_danger_squares;
  U64 attacks_on_king[2];

  int max_same_piece_in_row[64];
  int same_piece_in_row[64];
  U8 piece_in_row[64];

  int pst_score[16];

  U8 captured_history_list[32];
  U8 captured_history_ptr;
  U32 ply = 0;
};

void parse_FEN(Position* pos, const std::string& fen);
void print_board(Position* pos);
void print_bitboard_all(Position* pos);

void check_king_safety(Position* pos);

bool is_slider(Position* pos, int sq);

bool is_opening(Position* pos);
bool is_middle(Position* pos);
bool is_endgame(Position* pos);

#endif  // !__MUNCHKIN_POSITION_H__
