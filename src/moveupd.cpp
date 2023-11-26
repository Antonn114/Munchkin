#include "moveupd.h"

#include "position.h"

void do_move(Position* pos, Move m) {
  U8 from, to, flags;
  from = move_get_from(m);
  to = move_get_to(m);
  flags = move_get_flags(m);
  U8* mailbox = pos->board;
  U64* bitboard = pos->bb;
  U8* halfmove_clock = pos->halfmove_clock;
  U8* en_passant_sq = pos->en_passant_sq;
  U8 side_2_move = pos->side_2_move;
  U8* king_square = pos->king_square;
  pos->ply++;
  U32 ply = pos->ply;
  U8* captured_history_list = pos->captured_history_list;
  U8* castling_rights = pos->castling_rights;
  en_passant_sq[ply] = 64;
  castling_rights[ply] = castling_rights[ply - 1];
  halfmove_clock[ply] = halfmove_clock[ply - 1] + 1;
  switch (flags) {
    case MOVEFLAG_QUIET:
      if (board_to_bb(mailbox[from]) == mPawn) {
        pos->halfmove_clock[pos->ply] = 0;
      }
      break;
    case MOVEFLAG_PAWN_DPUSH:
      halfmove_clock[ply] = 0;
      en_passant_sq[ply] = to + (side_2_move == mWhite ? -8 : 8);
      break;
    case MOVEFLAG_CASTLE_KING:
      mailbox[king_square[side_2_move] + 1] =
          mailbox[king_square[side_2_move] + 3];
      mailbox[king_square[side_2_move] + 3] = 0;
      bitboard[side_2_move] ^= (1ULL << (king_square[side_2_move] + 1)) |
                               (1ULL << (king_square[side_2_move] + 3));
      bitboard[mRook] ^= (1ULL << (king_square[side_2_move] + 1)) |
                         (1ULL << (king_square[side_2_move] + 3));

      break;
    case MOVEFLAG_CASTLE_QUEEN:

      mailbox[king_square[side_2_move] - 1] =
          mailbox[king_square[side_2_move] - 4];
      mailbox[king_square[side_2_move] - 4] = 0;
      bitboard[side_2_move] ^= (1ULL << (king_square[side_2_move] - 1)) |
                               (1ULL << (king_square[side_2_move] - 4));
      bitboard[mRook] ^= (1ULL << (king_square[side_2_move] - 1)) |
                         (1ULL << (king_square[side_2_move] - 4));
      break;
    case MOVEFLAG_CAPTURE:
      halfmove_clock[ply] = 0;

      captured_history_list[pos->captured_history_ptr++] = mailbox[to];
      bitboard[1 ^ side_2_move] ^= (1ULL << (to));
      bitboard[board_to_bb(mailbox[to])] ^= (1ULL << (to));

      break;
    case MOVEFLAG_CAPTURE_EP:
      halfmove_clock[ply] = 0;
      captured_history_list[pos->captured_history_ptr++] =
          mailbox[en_passant_sq[ply - 1] + (side_2_move == mWhite ? -8 : 8)];
      mailbox[en_passant_sq[ply - 1] + (side_2_move == mWhite ? -8 : 8)] = 0;
      bitboard[1 ^ side_2_move] ^=
          (1ULL << (en_passant_sq[ply - 1] + (side_2_move == mWhite ? -8 : 8)));
      bitboard[mPawn] ^=
          (1ULL << (en_passant_sq[ply - 1] + (side_2_move == mWhite ? -8 : 8)));
      break;
    case MOVEFLAG_PROMOTE_KNIGHT:
      halfmove_clock[ply] = 0;
      bitboard[mPawn] ^= (1ULL << from);
      bitboard[mKnight] ^= (1ULL << from);
      mailbox[from] += -'P' + 'N';
      break;
    case MOVEFLAG_PROMOTE_BISHOP:
      halfmove_clock[ply] = 0;
      bitboard[mPawn] ^= (1ULL << from);
      bitboard[mBishop] ^= (1ULL << from);
      mailbox[from] += -'P' + 'B';
      break;
    case MOVEFLAG_PROMOTE_ROOK:
      halfmove_clock[ply] = 0;
      bitboard[mPawn] ^= (1ULL << from);
      bitboard[mRook] ^= (1ULL << from);
      mailbox[from] += -'P' + 'R';
      break;
    case MOVEFLAG_PROMOTE_QUEEN:
      halfmove_clock[ply] = 0;
      bitboard[mPawn] ^= (1ULL << from);
      bitboard[mQueen] ^= (1ULL << from);
      mailbox[from] += -'P' + 'Q';
      break;
    case MOVEFLAG_PROMOTE_KNIGHT_CAPTURE:
      halfmove_clock[ply] = 0;
      bitboard[mPawn] ^= (1ULL << from);
      bitboard[mKnight] ^= (1ULL << from);
      mailbox[from] += -'P' + 'N';
      captured_history_list[pos->captured_history_ptr++] = mailbox[to];
      bitboard[1 ^ side_2_move] ^= (1ULL << (to));
      bitboard[board_to_bb(mailbox[to])] ^= (1ULL << (to));
      break;
    case MOVEFLAG_PROMOTE_BISHOP_CAPTURE:
      halfmove_clock[ply] = 0;
      bitboard[mPawn] ^= (1ULL << from);
      bitboard[mBishop] ^= (1ULL << from);
      mailbox[from] += -'P' + 'B';
      captured_history_list[pos->captured_history_ptr++] = mailbox[to];
      bitboard[1 ^ side_2_move] ^= (1ULL << (to));
      bitboard[board_to_bb(mailbox[to])] ^= (1ULL << (to));
      break;
    case MOVEFLAG_PROMOTE_ROOK_CAPTURE:
      halfmove_clock[ply] = 0;
      bitboard[mPawn] ^= (1ULL << from);
      bitboard[mRook] ^= (1ULL << from);
      mailbox[from] += -'P' + 'R';
      captured_history_list[pos->captured_history_ptr++] = mailbox[to];
      bitboard[1 ^ side_2_move] ^= (1ULL << (to));
      bitboard[board_to_bb(mailbox[to])] ^= (1ULL << (to));
      break;
    case MOVEFLAG_PROMOTE_QUEEN_CAPTURE:
      halfmove_clock[ply] = 0;
      bitboard[mPawn] ^= (1ULL << from);
      bitboard[mQueen] ^= (1ULL << from);
      mailbox[from] += -'P' + 'Q';
      captured_history_list[pos->captured_history_ptr++] = mailbox[to];
      bitboard[1 ^ side_2_move] ^= (1ULL << (to));
      bitboard[board_to_bb(mailbox[to])] ^= (1ULL << (to));
      break;
  };
  if (board_to_bb(mailbox[to]) == mRook && castling_rights[ply]) {
    if (to == h1)
      castling_rights[ply] &= CASTLING_RIGHTS_WHITE_QUEEN |
                              CASTLING_RIGHTS_BLACK_KING |
                              CASTLING_RIGHTS_BLACK_QUEEN;
    if (to == a1)
      castling_rights[ply] &= CASTLING_RIGHTS_BLACK_QUEEN |
                              CASTLING_RIGHTS_BLACK_KING |
                              CASTLING_RIGHTS_WHITE_KING;
    if (to == h8)
      castling_rights[ply] &= CASTLING_RIGHTS_WHITE_KING |
                              CASTLING_RIGHTS_BLACK_QUEEN |
                              CASTLING_RIGHTS_WHITE_QUEEN;
    if (to == a8)
      castling_rights[ply] &= CASTLING_RIGHTS_WHITE_QUEEN |
                              CASTLING_RIGHTS_WHITE_KING |
                              CASTLING_RIGHTS_BLACK_KING;
  }
  if (board_to_bb(mailbox[from]) == mRook && castling_rights[ply]) {
    if (from == h1)
      castling_rights[ply] &= CASTLING_RIGHTS_WHITE_QUEEN |
                              CASTLING_RIGHTS_BLACK_KING |
                              CASTLING_RIGHTS_BLACK_QUEEN;
    if (from == a1)
      castling_rights[ply] &= CASTLING_RIGHTS_BLACK_QUEEN |
                              CASTLING_RIGHTS_BLACK_KING |
                              CASTLING_RIGHTS_WHITE_KING;
    if (from == h8)
      castling_rights[ply] &= CASTLING_RIGHTS_WHITE_KING |
                              CASTLING_RIGHTS_BLACK_QUEEN |
                              CASTLING_RIGHTS_WHITE_QUEEN;
    if (from == a8)
      castling_rights[ply] &= CASTLING_RIGHTS_WHITE_QUEEN |
                              CASTLING_RIGHTS_WHITE_KING |
                              CASTLING_RIGHTS_BLACK_KING;
  }
  bitboard[side_2_move] ^= (1ULL << to) | (1ULL << from);
  bitboard[board_to_bb(mailbox[from])] ^= (1ULL << to) | (1ULL << from);
  mailbox[to] = mailbox[from];
  mailbox[from] = 0;
  if (king_square[side_2_move] == from) {
    king_square[side_2_move] = to;
    if (side_2_move == mWhite)
      pos->castling_rights[ply] &=
          (CASTLING_RIGHTS_BLACK_KING | CASTLING_RIGHTS_BLACK_QUEEN);
    else
      pos->castling_rights[ply] &=
          (CASTLING_RIGHTS_WHITE_KING | CASTLING_RIGHTS_WHITE_QUEEN);
  }

  pos->side_2_move ^= 1;
}

void undo_move(Position* pos, Move m) {
  pos->side_2_move ^= 1;

  U8* mailbox = pos->board;
  U64* bitboard = pos->bb;
  U8* en_passant_sq = pos->en_passant_sq;
  U8 side_2_move = pos->side_2_move;
  U8* king_square = pos->king_square;
  U32 ply = pos->ply;
  U8* captured_history_list = pos->captured_history_list;

  U8 from, to, flags;
  from = move_get_from(m);
  to = move_get_to(m);
  flags = move_get_flags(m);
  if (king_square[side_2_move] == to) {
    king_square[side_2_move] = from;
  }
  mailbox[from] = mailbox[to];
  mailbox[to] = 0;
  bitboard[side_2_move] ^= (1ULL << to) | (1ULL << from);
  bitboard[board_to_bb(mailbox[from])] ^= (1ULL << to) | (1ULL << from);
  switch (flags) {
    case MOVEFLAG_CASTLE_KING:
      mailbox[king_square[side_2_move] + 3] =
          mailbox[king_square[side_2_move] + 1];
      mailbox[king_square[side_2_move] + 1] = 0;

      bitboard[side_2_move] ^= (1ULL << (king_square[side_2_move] + 1)) |
                               (1ULL << (king_square[side_2_move] + 3));
      bitboard[mRook] ^= (1ULL << (king_square[side_2_move] + 1)) |
                         (1ULL << (king_square[side_2_move] + 3));
      break;
    case MOVEFLAG_CASTLE_QUEEN:
      mailbox[king_square[side_2_move] - 4] =
          mailbox[king_square[side_2_move] - 1];
      mailbox[king_square[side_2_move] - 1] = 0;
      bitboard[side_2_move] ^= (1ULL << (king_square[side_2_move] - 1)) |
                               (1ULL << (king_square[side_2_move] - 4));
      bitboard[mRook] ^= (1ULL << (king_square[side_2_move] - 1)) |
                         (1ULL << (king_square[side_2_move] - 4));
      break;
    case MOVEFLAG_CAPTURE:
      pos->captured_history_ptr--;
      mailbox[to] = captured_history_list[pos->captured_history_ptr];
      bitboard[board_to_bb(captured_history_list[pos->captured_history_ptr])] ^=
          (1ULL << (to));
      bitboard[1 ^ side_2_move] ^= (1ULL << (to));
      break;
    case MOVEFLAG_CAPTURE_EP:
      pos->captured_history_ptr--;
      mailbox[en_passant_sq[ply - 1] + (side_2_move == mWhite ? -8 : 8)] =
          captured_history_list[pos->captured_history_ptr];
      bitboard[1 ^ side_2_move] ^=
          (1ULL << (en_passant_sq[ply - 1] + (side_2_move == mWhite ? -8 : 8)));
      bitboard[mPawn] ^=
          (1ULL << (en_passant_sq[ply - 1] + (side_2_move == mWhite ? -8 : 8)));
      break;
    case MOVEFLAG_PROMOTE_KNIGHT:
      bitboard[mPawn] ^= (1ULL << from);
      bitboard[mKnight] ^= (1ULL << from);
      mailbox[from] -= -'P' + 'N';
      break;
    case MOVEFLAG_PROMOTE_BISHOP:
      bitboard[mPawn] ^= (1ULL << from);
      bitboard[mBishop] ^= (1ULL << from);
      mailbox[from] -= -'P' + 'B';
      break;
    case MOVEFLAG_PROMOTE_ROOK:
      bitboard[mPawn] ^= (1ULL << from);
      bitboard[mRook] ^= (1ULL << from);
      mailbox[from] -= -'P' + 'R';
      break;
    case MOVEFLAG_PROMOTE_QUEEN:
      bitboard[mPawn] ^= (1ULL << from);
      bitboard[mQueen] ^= (1ULL << from);
      mailbox[from] -= -'P' + 'Q';
      break;
    case MOVEFLAG_PROMOTE_KNIGHT_CAPTURE:
      bitboard[mPawn] ^= (1ULL << from);
      bitboard[mKnight] ^= (1ULL << from);
      mailbox[from] -= -'P' + 'N';
      pos->captured_history_ptr--;
      mailbox[to] = captured_history_list[pos->captured_history_ptr];
      bitboard[1 ^ side_2_move] ^= (1ULL << (to));

      bitboard[board_to_bb(captured_history_list[pos->captured_history_ptr])] ^=
          (1ULL << (to));
      break;
    case MOVEFLAG_PROMOTE_BISHOP_CAPTURE:
      bitboard[mPawn] ^= (1ULL << from);
      bitboard[mBishop] ^= (1ULL << from);
      mailbox[from] -= -'P' + 'B';
      pos->captured_history_ptr--;
      mailbox[to] = captured_history_list[pos->captured_history_ptr];
      bitboard[1 ^ side_2_move] ^= (1ULL << (to));

      bitboard[board_to_bb(captured_history_list[pos->captured_history_ptr])] ^=
          (1ULL << (to));
      break;
    case MOVEFLAG_PROMOTE_ROOK_CAPTURE:
      bitboard[mPawn] ^= (1ULL << from);
      bitboard[mRook] ^= (1ULL << from);
      mailbox[from] -= -'P' + 'R';
      pos->captured_history_ptr--;
      mailbox[to] = captured_history_list[pos->captured_history_ptr];
      bitboard[1 ^ side_2_move] ^= (1ULL << (to));

      bitboard[board_to_bb(captured_history_list[pos->captured_history_ptr])] ^=
          (1ULL << (to));
      break;
    case MOVEFLAG_PROMOTE_QUEEN_CAPTURE:
      bitboard[mPawn] ^= (1ULL << from);
      bitboard[mQueen] ^= (1ULL << from);
      mailbox[from] -= -'P' + 'Q';
      pos->captured_history_ptr--;
      mailbox[to] = captured_history_list[pos->captured_history_ptr];
      bitboard[1 ^ side_2_move] ^= (1ULL << (to));

      bitboard[board_to_bb(captured_history_list[pos->captured_history_ptr])] ^=
          (1ULL << (to));
      break;
  };
  pos->ply--;
}
