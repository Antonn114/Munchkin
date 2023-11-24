#include "position.h"

#include "defs.h"

U8 board_to_bb(U8 c) {
  return (c == WHITE_PAWN_CH || c == BLACK_PAWN_CH) * mPawn |
         (c == WHITE_KNIGHT_CH || c == BLACK_KNIGHT_CH) * mKnight |
         (c == WHITE_BISHOP_CH || c == BLACK_BISHOP_CH) * mBishop |
         (c == WHITE_ROOK_CH || c == BLACK_ROOK_CH) * mRook |
         (c == WHITE_QUEEN_CH || c == BLACK_QUEEN_CH) * mQueen |
         (c == WHITE_KING_CH || c == BLACK_KING_CH) * mKing;
}

U8 bb_to_board(U8 c, U8 p) {
  if (c == mWhite) {
    return (p == mPawn) * WHITE_PAWN_CH | (p == mKnight) * WHITE_KNIGHT_CH |
           (p == mBishop) * WHITE_BISHOP_CH | (p == mRook) * WHITE_ROOK_CH |
           (p == mQueen) * WHITE_QUEEN_CH | (p == mKing) * WHITE_KING_CH;
  } else {
    return (p == mPawn) * BLACK_PAWN_CH | (p == mKnight) * BLACK_KNIGHT_CH |
           (p == mBishop) * WHITE_BISHOP_CH | (p == mRook) * WHITE_ROOK_CH |
           (p == mQueen) * WHITE_QUEEN_CH | (p == mKing) * WHITE_KING_CH;
  }
}

void parse_FEN(Position* pos, const std::string& fen) {
  U8 board_ptr = 56;
  pos->ply = 0;
  for (U8 i = 0; i < 64; i++) {
    pos->board[i] = 0;
  }
  for (U8 i = 0; i < 8; i++) {
    pos->bb[i] = 0;
  }
  int c_i = 0;
  int cnt = 0;
  for (; c_i < (int)fen.length() && cnt < 64; c_i++) {
    if (isspace(fen[c_i]))
      continue;
    else if (isdigit(fen[c_i])) {
      board_ptr += fen[c_i] - '0';
      cnt += fen[c_i] - '0';
    } else if (fen[c_i] == '/') {
      board_ptr -= 16;
    } else {
      if (fen[c_i] == 'k') pos->king_square[mBlack] = board_ptr;
      if (fen[c_i] == 'K') pos->king_square[mWhite] = board_ptr;
      pos->board[board_ptr] = fen[c_i];
      pos->bb[islower(fen[c_i]) ? mBlack : mWhite] |= (1ULL << board_ptr);
      pos->bb[board_to_bb(fen[c_i])] |= (1ULL << board_ptr);
      board_ptr++;
      cnt++;
    }
  }
  while (isspace(fen[c_i])) {
    c_i++;
  }
  pos->side_2_move = (fen[c_i] == 'w' ? mWhite : mBlack);
  c_i++;
  while (isspace(fen[c_i])) {
    c_i++;
  }
  pos->castling_rights[pos->ply] = 0;
  if (fen[c_i] == '-')
    c_i++;
  else {
    while (!isspace(fen[c_i])) {
      if (fen[c_i] == 'K')
        pos->castling_rights[pos->ply] |= CASTLING_RIGHTS_WHITE_KING;
      else if (fen[c_i] == 'k')
        pos->castling_rights[pos->ply] |= CASTLING_RIGHTS_BLACK_KING;
      else if (fen[c_i] == 'Q')
        pos->castling_rights[pos->ply] |= CASTLING_RIGHTS_WHITE_QUEEN;
      else if (fen[c_i] == 'q')
        pos->castling_rights[pos->ply] |= CASTLING_RIGHTS_BLACK_QUEEN;
      c_i++;
    }
    c_i++;
  }
  while (isspace(fen[c_i])) {
    c_i++;
  }
  pos->en_passant_sq[pos->ply] = 0;
  if (fen[c_i] == '-')
    c_i++;
  else {
    pos->en_passant_sq[pos->ply] = (fen[c_i] - 'a') * 8 + fen[c_i + 1] - '0';
    c_i += 2;
  }
  while (isspace(fen[c_i])) {
    c_i++;
  }
  pos->halfmove_clock[pos->ply] =
      isspace(fen[c_i + 1]) ? fen[c_i] - '0'
                            : (fen[c_i] - '0') * 10 + fen[c_i + 1] - '0';
}
