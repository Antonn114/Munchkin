#include "moveupd.h"

#include <assert.h>

#include <iostream>

#include "bitboard.h"
#include "evaluation.h"
#include "position.h"

void remove_piece(Position* pos, U8 piece, U8 col, U8 sq) {
  assert((pos->bb[col] >> sq) & 1);
  assert((pos->bb[piece] >> sq) & 1);
  pos->board[sq] = 0;
  pos->bb[col] ^= (1ULL << sq);
  pos->bb[piece] ^= (1ULL << sq);
  if (piece != mKing) {
    pos->pst_score[piece + col * 8] -=
        piece_square_table[piece][col == mBlack ? flipped_square(sq) : sq];
  } else {
    pos->pst_score[piece + col * 8] -=
        (is_endgame(pos)
             ? (pst_King_Black_endgame[col == mBlack ? flipped_square(sq) : sq])
             : pst_King_Black_middlegame[col == mBlack ? flipped_square(sq)
                                                       : sq]);
    if (col == mWhite) {
      pos->castling_rights[pos->ply] &=
          CASTLING_RIGHTS_BLACK_KING | CASTLING_RIGHTS_BLACK_QUEEN;
    } else
      pos->castling_rights[pos->ply] &=
          CASTLING_RIGHTS_WHITE_KING | CASTLING_RIGHTS_WHITE_QUEEN;
  }
  if (piece == mRook) {
    if (sq == a1 && col == mWhite)
      pos->castling_rights[pos->ply] &=
          (CASTLING_RIGHTS_WHITE_KING | CASTLING_RIGHTS_BLACK_QUEEN |
           CASTLING_RIGHTS_BLACK_KING);
    else if (sq == h1 && col == mWhite)
      pos->castling_rights[pos->ply] &=
          (CASTLING_RIGHTS_WHITE_QUEEN | CASTLING_RIGHTS_BLACK_QUEEN |
           CASTLING_RIGHTS_BLACK_KING);
    else if (sq == a8 && col == mBlack)
      pos->castling_rights[pos->ply] &=
          (CASTLING_RIGHTS_WHITE_KING | CASTLING_RIGHTS_WHITE_QUEEN |
           CASTLING_RIGHTS_BLACK_KING);
    else if (sq == h8 && col == mBlack)
      pos->castling_rights[pos->ply] &=
          (CASTLING_RIGHTS_WHITE_KING | CASTLING_RIGHTS_WHITE_QUEEN |
           CASTLING_RIGHTS_BLACK_QUEEN);
  }
}

void add_piece(Position* pos, U8 piece, U8 col, U8 sq) {
  assert((pos->bb[col] >> sq) & 1 ^ 1);
  assert((pos->bb[piece] >> sq) & 1 ^ 1);
  pos->board[sq] = bb_to_board(col, piece);
  pos->bb[col] ^= (1ULL << sq);
  pos->bb[piece] ^= (1ULL << sq);
  if (piece != mKing)
    pos->pst_score[piece + col * 8] +=
        piece_square_table[piece][col == mBlack ? flipped_square(sq) : sq];
  else {
    pos->pst_score[piece + col * 8] +=
        (is_endgame(pos)
             ? (pst_King_Black_endgame[col == mBlack ? flipped_square(sq) : sq])
             : pst_King_Black_middlegame[col == mBlack ? flipped_square(sq)
                                                       : sq]);
    pos->king_square[col] = sq;
  }
}

void do_move(Position* pos, Move m) {
  U8 from, to, flags;
  from = move_get_from(m);
  to = move_get_to(m);
  flags = move_get_flags(m);
  pos->ply++;
  pos->en_passant_sq[pos->ply] = 64;
  pos->castling_rights[pos->ply] = pos->castling_rights[pos->ply - 1];
  pos->halfmove_clock[pos->ply] = pos->halfmove_clock[pos->ply - 1] + 1;
  switch (flags) {
    case MOVEFLAG_QUIET:
      if (board_to_bb(pos->board[from]) == mPawn) {
        pos->halfmove_clock[pos->ply] = 0;
      }
      break;
    case MOVEFLAG_PAWN_DPUSH:
      pos->halfmove_clock[pos->ply] = 0;
      pos->en_passant_sq[pos->ply] = to + (pos->side_2_move == mWhite ? -8 : 8);
      break;
    case MOVEFLAG_CASTLE_KING:
      remove_piece(pos, mRook, pos->side_2_move,
                   pos->king_square[pos->side_2_move] + 3);
      add_piece(pos, mRook, pos->side_2_move,
                pos->king_square[pos->side_2_move] + 1);
      break;
    case MOVEFLAG_CASTLE_QUEEN:
      remove_piece(pos, mRook, pos->side_2_move,
                   pos->king_square[pos->side_2_move] - 4);
      add_piece(pos, mRook, pos->side_2_move,
                pos->king_square[pos->side_2_move] - 1);
      break;
    case MOVEFLAG_CAPTURE:
      pos->halfmove_clock[pos->ply] = 0;

      pos->captured_history_list[pos->captured_history_ptr++] = pos->board[to];
      remove_piece(pos, board_to_bb(pos->board[to]), 1 ^ pos->side_2_move, to);
      break;
    case MOVEFLAG_CAPTURE_EP:
      pos->halfmove_clock[pos->ply] = 0;
      pos->captured_history_list[pos->captured_history_ptr++] =
          pos->board[pos->en_passant_sq[pos->ply - 1] +
                     (pos->side_2_move == mWhite ? -8 : 8)];
      remove_piece(pos, mPawn, 1 ^ pos->side_2_move,
                   pos->en_passant_sq[pos->ply - 1] +
                       (pos->side_2_move == mWhite ? -8 : 8));
      break;
    case MOVEFLAG_PROMOTE_KNIGHT:
      pos->halfmove_clock[pos->ply] = 0;
      remove_piece(pos, mPawn, pos->side_2_move, from);
      add_piece(pos, mKnight, pos->side_2_move, from);
      break;
    case MOVEFLAG_PROMOTE_BISHOP:
      pos->halfmove_clock[pos->ply] = 0;
      remove_piece(pos, mPawn, pos->side_2_move, from);
      add_piece(pos, mBishop, pos->side_2_move, from);
      break;
    case MOVEFLAG_PROMOTE_ROOK:
      pos->halfmove_clock[pos->ply] = 0;
      remove_piece(pos, mPawn, pos->side_2_move, from);
      add_piece(pos, mRook, pos->side_2_move, from);
      break;
    case MOVEFLAG_PROMOTE_QUEEN:
      pos->halfmove_clock[pos->ply] = 0;
      remove_piece(pos, mPawn, pos->side_2_move, from);
      add_piece(pos, mQueen, pos->side_2_move, from);
      break;
    case MOVEFLAG_PROMOTE_KNIGHT_CAPTURE:
      pos->halfmove_clock[pos->ply] = 0;
      remove_piece(pos, mPawn, pos->side_2_move, from);
      add_piece(pos, mKnight, pos->side_2_move, from);
      pos->captured_history_list[pos->captured_history_ptr++] = pos->board[to];
      remove_piece(pos, board_to_bb(pos->board[to]), 1 ^ pos->side_2_move, to);
      break;
    case MOVEFLAG_PROMOTE_BISHOP_CAPTURE:
      pos->halfmove_clock[pos->ply] = 0;
      remove_piece(pos, mPawn, pos->side_2_move, from);
      add_piece(pos, mBishop, pos->side_2_move, from);
      pos->captured_history_list[pos->captured_history_ptr++] = pos->board[to];
      remove_piece(pos, board_to_bb(pos->board[to]), 1 ^ pos->side_2_move, to);
      break;
    case MOVEFLAG_PROMOTE_ROOK_CAPTURE:
      pos->halfmove_clock[pos->ply] = 0;
      remove_piece(pos, mPawn, pos->side_2_move, from);
      add_piece(pos, mRook, pos->side_2_move, from);
      pos->captured_history_list[pos->captured_history_ptr++] = pos->board[to];
      remove_piece(pos, board_to_bb(pos->board[to]), 1 ^ pos->side_2_move, to);
      break;
    case MOVEFLAG_PROMOTE_QUEEN_CAPTURE:
      pos->halfmove_clock[pos->ply] = 0;
      remove_piece(pos, mPawn, pos->side_2_move, from);
      add_piece(pos, mQueen, pos->side_2_move, from);
      pos->captured_history_list[pos->captured_history_ptr++] = pos->board[to];
      remove_piece(pos, board_to_bb(pos->board[to]), 1 ^ pos->side_2_move, to);
      break;
  };
  U8 piece = pos->board[from];
  remove_piece(pos, board_to_bb(piece), pos->side_2_move, from);
  add_piece(pos, board_to_bb(piece), pos->side_2_move, to);

  pos->side_2_move ^= 1;
  if (pos->side_2_move == mWhite) pos->fullmove++;
}

void undo_move(Position* pos, Move m) {
  if (pos->side_2_move == mWhite) pos->fullmove--;
  pos->side_2_move ^= 1;

  U8 from, to, flags;
  from = move_get_from(m);
  to = move_get_to(m);
  flags = move_get_flags(m);
  U8 piece = pos->board[to];
  remove_piece(pos, board_to_bb(piece), pos->side_2_move, to);
  add_piece(pos, board_to_bb(piece), pos->side_2_move, from);

  switch (flags) {
    case MOVEFLAG_CASTLE_KING:
      remove_piece(pos, mRook, pos->side_2_move,
                   pos->king_square[pos->side_2_move] + 1);
      add_piece(pos, mRook, pos->side_2_move,
                pos->king_square[pos->side_2_move] + 3);
      break;
    case MOVEFLAG_CASTLE_QUEEN:
      remove_piece(pos, mRook, pos->side_2_move,
                   pos->king_square[pos->side_2_move] - 1);
      add_piece(pos, mRook, pos->side_2_move,
                pos->king_square[pos->side_2_move] - 4);
      break;
    case MOVEFLAG_CAPTURE:
      pos->captured_history_ptr--;
      add_piece(
          pos,
          board_to_bb(pos->captured_history_list[pos->captured_history_ptr]),
          1 ^ pos->side_2_move, to);
      break;
    case MOVEFLAG_CAPTURE_EP:
      pos->captured_history_ptr--;
      add_piece(pos, mPawn, 1 ^ pos->side_2_move,
                pos->en_passant_sq[pos->ply - 1] +
                    (pos->side_2_move == mWhite ? -8 : 8));
      break;
    case MOVEFLAG_PROMOTE_KNIGHT:
      remove_piece(pos, mKnight, pos->side_2_move, from);
      add_piece(pos, mPawn, pos->side_2_move, from);
      break;
    case MOVEFLAG_PROMOTE_BISHOP:
      remove_piece(pos, mBishop, pos->side_2_move, from);
      add_piece(pos, mPawn, pos->side_2_move, from);
      break;
    case MOVEFLAG_PROMOTE_ROOK:
      remove_piece(pos, mRook, pos->side_2_move, from);
      add_piece(pos, mPawn, pos->side_2_move, from);
      break;
    case MOVEFLAG_PROMOTE_QUEEN:
      remove_piece(pos, mQueen, pos->side_2_move, from);
      add_piece(pos, mPawn, pos->side_2_move, from);
      break;
    case MOVEFLAG_PROMOTE_KNIGHT_CAPTURE:
      remove_piece(pos, mKnight, pos->side_2_move, from);
      add_piece(pos, mPawn, pos->side_2_move, from);
      pos->captured_history_ptr--;
      add_piece(
          pos,
          board_to_bb(pos->captured_history_list[pos->captured_history_ptr]),
          1 ^ pos->side_2_move, to);
      break;
    case MOVEFLAG_PROMOTE_BISHOP_CAPTURE:
      remove_piece(pos, mBishop, pos->side_2_move, from);
      add_piece(pos, mPawn, pos->side_2_move, from);
      pos->captured_history_ptr--;
      add_piece(
          pos,
          board_to_bb(pos->captured_history_list[pos->captured_history_ptr]),
          1 ^ pos->side_2_move, to);
      break;
    case MOVEFLAG_PROMOTE_ROOK_CAPTURE:
      remove_piece(pos, mRook, pos->side_2_move, from);
      add_piece(pos, mPawn, pos->side_2_move, from);
      pos->captured_history_ptr--;
      add_piece(
          pos,
          board_to_bb(pos->captured_history_list[pos->captured_history_ptr]),
          1 ^ pos->side_2_move, to);
      break;
    case MOVEFLAG_PROMOTE_QUEEN_CAPTURE:
      remove_piece(pos, mQueen, pos->side_2_move, from);
      add_piece(pos, mPawn, pos->side_2_move, from);
      pos->captured_history_ptr--;
      add_piece(
          pos,
          board_to_bb(pos->captured_history_list[pos->captured_history_ptr]),
          1 ^ pos->side_2_move, to);
      break;
  };
  pos->ply--;
}
