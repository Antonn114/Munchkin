#include "movegen.h"

#include <string.h>

#include "bitboard.h"
#include "bittricks.h"

Move create_move(U8 from, U8 to, U8 flags) {
  return ((flags & 0xf) << 12) | ((from & 0x3f) << 6) | (to & 0x3f);
}

U8 move_get_flags(Move m) { return (m >> 12) & 0x0f; }

U8 move_get_from(Move m) { return (m >> 6) & 0x3f; }

U8 move_get_to(Move m) { return m & 0x3f; }

void movelist_add_promotion(int sq, int next_sq, Move* move_list, U8& move_ptr,
                            bool is_capture) {
  move_list[move_ptr++] = create_move(
      sq, next_sq,
      is_capture ? MOVEFLAG_PROMOTE_KNIGHT_CAPTURE : MOVEFLAG_PROMOTE_KNIGHT);
  move_list[move_ptr++] = create_move(
      sq, next_sq,
      is_capture ? MOVEFLAG_PROMOTE_BISHOP_CAPTURE : MOVEFLAG_PROMOTE_BISHOP);
  move_list[move_ptr++] = create_move(
      sq, next_sq,
      is_capture ? MOVEFLAG_PROMOTE_ROOK_CAPTURE : MOVEFLAG_PROMOTE_ROOK);
  move_list[move_ptr++] = create_move(
      sq, next_sq,
      is_capture ? MOVEFLAG_PROMOTE_QUEEN_CAPTURE : MOVEFLAG_PROMOTE_QUEEN);
}
void gen_rook_moves(Position* pos, int sq, Move move_list[], U8& move_ptr) {
  U64 good_mask = get_rook_attacks(
      pos->bb[1 ^ pos->side_2_move] | pos->bb[pos->side_2_move], sq);

  good_mask &= ~pos->bb[pos->side_2_move];
  good_mask &= pos->capture_mask | pos->push_mask;
  int next_sq;
  while (good_mask) {
    next_sq = bitScanForward(good_mask);
    move_list[move_ptr++] = create_move(
        sq, next_sq,
        ((pos->bb[1 ^ pos->side_2_move] >> next_sq) & 1) * MOVEFLAG_CAPTURE);
    good_mask ^= (1ULL << next_sq);
  }
}

void gen_bishop_moves(Position* pos, int sq, Move* move_list, U8& move_ptr) {
  U64 good_mask = get_bishop_attacks(
      pos->bb[1 ^ pos->side_2_move] | pos->bb[pos->side_2_move], sq);
  good_mask &= ~pos->bb[pos->side_2_move];
  good_mask &= pos->capture_mask | pos->push_mask;
  int next_sq;
  while (good_mask) {
    next_sq = bitScanForward(good_mask);
    move_list[move_ptr++] = create_move(
        sq, next_sq,
        ((pos->bb[1 ^ pos->side_2_move] >> next_sq) & 1) * MOVEFLAG_CAPTURE);
    good_mask ^= (1ULL << next_sq);
  }
}

void gen_knight_moves(Position* pos, int sq, Move* move_list, U8& move_ptr) {
  U64 good_mask = knight_attacks[sq];
  good_mask &= ~pos->bb[pos->side_2_move];
  good_mask &= pos->capture_mask | pos->push_mask;
  int next_sq;
  while (good_mask) {
    next_sq = bitScanForward(good_mask);
    move_list[move_ptr++] = create_move(
        sq, next_sq,
        ((pos->bb[1 ^ pos->side_2_move] >> next_sq) & 1) * MOVEFLAG_CAPTURE);
    good_mask ^= (1ULL << next_sq);
  }
}

void gen_pawn_capture_moves(Position* pos, int sq, Move* move_list,
                            U8& move_ptr) {
  // normal diag capture (no en passant)
  U64 good_mask = pawn_attacks[pos->side_2_move][sq];
  good_mask &= ~pos->bb[pos->side_2_move];
  good_mask &= pos->bb[1 ^ pos->side_2_move];
  good_mask &= pos->capture_mask | pos->push_mask;
  int next_sq;
  while (good_mask) {
    next_sq = bitScanForward(good_mask);
    if (((sq >> 3) == 6 && pos->side_2_move == mWhite) ||
        ((sq >> 3) == 1 && pos->side_2_move == mBlack)) {
      movelist_add_promotion(sq, next_sq, move_list, move_ptr, true);
    } else
      move_list[move_ptr++] = create_move(sq, next_sq, MOVEFLAG_CAPTURE);
    good_mask ^= (1ULL << next_sq);
  }
  // en passant
  if (pos->en_passant_sq[pos->ply] > 63) return;
  good_mask = pawn_attacks[pos->side_2_move][sq];
  good_mask &= (1ULL << pos->en_passant_sq[pos->ply]);
  if ((pos->attacks_on_king[pos->side_2_move] &
           (1ULL << (pos->en_passant_sq[pos->ply] +
                     (pos->side_2_move == mWhite ? -8 : 8))) &&
       good_mask) ||
      (!pos->attacks_on_king[pos->side_2_move] && good_mask)) {
    // checking that one position
    U64 company =
        (1ULL << sq) | (1ULL << (pos->en_passant_sq[pos->ply] +
                                 (pos->side_2_move == mWhite ? -8 : 8)));
    U64 funny = get_rook_attacks(
        (pos->bb[1 ^ pos->side_2_move] | pos->bb[pos->side_2_move]) ^ company,
        pos->king_square[pos->side_2_move]);
    funny &= west_attacks[pos->king_square[pos->side_2_move]] |
             east_attacks[pos->king_square[pos->side_2_move]];
    funny &= ~pos->bb[pos->side_2_move];
    if (!(funny & (pos->bb[mRook] | pos->bb[mQueen]))) {
      move_list[move_ptr++] =
          create_move(sq, pos->en_passant_sq[pos->ply], MOVEFLAG_CAPTURE_EP);
    }
  }
}

void gen_pawn_push_moves(Position* pos, int sq, Move* move_list, U8& move_ptr) {
  U64 good_mask = (1ULL << sq);
  if (pos->side_2_move == mWhite)
    good_mask <<= 8;
  else
    good_mask >>= 8;
  good_mask &= ~pos->bb[pos->side_2_move];
  good_mask &= ~pos->bb[1 ^ pos->side_2_move];
  bool cant_push = (good_mask == 0);
  good_mask &= pos->push_mask;
  if (good_mask) {
    if (((sq >> 3) == 6 && pos->side_2_move == mWhite) ||
        ((sq >> 3) == 1 && pos->side_2_move == mBlack)) {
      movelist_add_promotion(sq, sq + (pos->side_2_move == mWhite ? 8 : -8),
                             move_list, move_ptr, false);
    } else
      move_list[move_ptr++] = create_move(
          sq, sq + (pos->side_2_move == mWhite ? 8 : -8), MOVEFLAG_QUIET);
  }
  // double push
  if (cant_push) return;
  good_mask = (1ULL << sq);
  if (pos->side_2_move == mWhite && (sq >> 3) == 1) {
    good_mask <<= 16;
  } else if (pos->side_2_move == mBlack && (sq >> 3) == 6) {
    good_mask >>= 16;
  } else
    return;
  good_mask &= ~pos->bb[pos->side_2_move];
  good_mask &= ~pos->bb[1 ^ pos->side_2_move];
  good_mask &= pos->push_mask;
  if (good_mask) {
    move_list[move_ptr++] = create_move(
        sq, sq + (pos->side_2_move == mBlack ? -16 : 16), MOVEFLAG_PAWN_DPUSH);
  }
}

void gen_pawn_moves(Position* pos, int sq, Move* move_list, U8& move_ptr) {
  gen_pawn_capture_moves(pos, sq, move_list, move_ptr);
  gen_pawn_push_moves(pos, sq, move_list, move_ptr);
}

void gen_king_moves(Position* pos, int sq, Move* move_list, U8& move_ptr) {
  // normal movement
  U64 good_mask = king_attacks[sq];
  good_mask &= ~pos->bb[pos->side_2_move];
  good_mask &= ~pos->king_danger_squares;

  int next_sq;
  while (good_mask) {
    next_sq = bitScanForward(good_mask);
    move_list[move_ptr++] = create_move(
        sq, next_sq,
        ((pos->bb[1 ^ pos->side_2_move] >> next_sq) & 1) * MOVEFLAG_CAPTURE);
    good_mask ^= (1ULL << next_sq);
  }
  if (pos->attacks_on_king[pos->side_2_move]) return;
  // castling : king
  if (((pos->side_2_move == mWhite &&
        (pos->castling_rights[pos->ply] & CASTLING_RIGHTS_WHITE_KING)) ||
       (pos->side_2_move == mBlack &&
        (pos->castling_rights[pos->ply] & CASTLING_RIGHTS_BLACK_KING))) &
          (pos->board[sq + 1] == 0 && pos->board[sq + 2] == 0) &&
      ((pos->king_danger_squares >> (sq + 1)) & 1) ^ 1 &&
      ((pos->king_danger_squares >> (sq + 2)) & 1) ^ 1) {
    move_list[move_ptr++] = create_move(sq, sq + 2, MOVEFLAG_CASTLE_KING);
  }
  // castling : queen
  if (((pos->side_2_move == mWhite &&
        (pos->castling_rights[pos->ply] & CASTLING_RIGHTS_WHITE_QUEEN)) ||
       (pos->side_2_move == mBlack &&
        (pos->castling_rights[pos->ply] & CASTLING_RIGHTS_BLACK_QUEEN))) &&
      (pos->board[sq - 1] == 0 && pos->board[sq - 2] == 0 &&
       pos->board[sq - 3] == 0) &&
      ((pos->king_danger_squares >> (sq - 1)) & 1) ^ 1 &&
      ((pos->king_danger_squares >> (sq - 2)) & 1) ^ 1) {
    move_list[move_ptr++] = create_move(sq, sq - 2, MOVEFLAG_CASTLE_QUEEN);
  }
}

void gen_pinned_pieces_moves_diag(Position* pos, Move* move_list,
                                  U8& move_ptr) {
  U64 m =
      pos->pinned_mask & (pos->bb[mBishop] | pos->bb[mQueen] | pos->bb[mPawn]);

  U8 sq;
  while (m) {
    sq = bitScanForward(m);
    U64 good_mask = get_bishop_attacks(
        pos->bb[pos->side_2_move] | pos->bb[1 ^ pos->side_2_move], sq);
    good_mask &= pos->capture_mask | pos->push_mask;

    if (((noea_attacks[sq] | sowe_attacks[sq]) >>
         pos->king_square[pos->side_2_move]) &
        1) {
      good_mask &= (noea_attacks[sq] | sowe_attacks[sq]);
    } else if (((soea_attacks[sq] | nowe_attacks[sq]) >>
                pos->king_square[pos->side_2_move]) &
               1) {
      good_mask &= (soea_attacks[sq] | nowe_attacks[sq]);
    } else {
      good_mask &= 0;
    }
    if ((pos->bb[mPawn] >> sq) & 1) {
      good_mask &= pawn_attacks[pos->side_2_move][sq];
      good_mask &= pos->bb[1 ^ pos->side_2_move];
    } else {
      good_mask &= ~pos->bb[pos->side_2_move];
    }
    int next_sq;
    while (good_mask) {
      next_sq = bitScanForward(good_mask);
      if (((pos->bb[mPawn] >> sq) & 1) &&
          (((sq >> 3) == 6 && pos->side_2_move == mWhite) ||
           ((sq >> 3) == 1 && pos->side_2_move == mBlack))) {
        movelist_add_promotion(sq, next_sq, move_list, move_ptr, true);
      } else
        move_list[move_ptr++] = create_move(
            sq, next_sq,
            (((pos->bb[1 ^ pos->side_2_move] >> next_sq) & 1) << 2));
      good_mask ^= (1ULL << next_sq);
    }
    m ^= (1ULL << sq);
  }
}

void gen_pinned_pieces_moves_nondiag(Position* pos, Move* move_list,
                                     U8& move_ptr) {
  U64 m =
      pos->pinned_mask & (pos->bb[mRook] | pos->bb[mQueen] | pos->bb[mPawn]);
  U8 sq;
  while (m) {
    sq = bitScanForward(m);
    U64 good_mask = get_rook_attacks(
        pos->bb[pos->side_2_move] | pos->bb[1 ^ pos->side_2_move], sq);
    good_mask &= pos->capture_mask | pos->push_mask;
    if (((nort_attacks[sq] | sout_attacks[sq]) >>
         pos->king_square[pos->side_2_move]) &
        1) {
      good_mask &= (nort_attacks[sq] | sout_attacks[sq]);
    } else if (((west_attacks[sq] | east_attacks[sq]) >>
                pos->king_square[pos->side_2_move]) &
               1) {
      good_mask &= (east_attacks[sq] | west_attacks[sq]);
    } else {
      good_mask &= 0;
    }
    if ((pos->bb[mPawn] >> sq) & 1) {
      if (pos->side_2_move == mWhite) {
        good_mask &= nort_attacks[sq];
        if ((sq >> 3) == 1)
          good_mask &= C64(0xffffffff);
        else
          good_mask &= ((1ULL << sq) << 8);
      } else {
        good_mask &= sout_attacks[sq];
        if ((sq >> 3) == 6)
          good_mask &= C64(0xffffffff00000000);
        else
          good_mask &= ((1ULL << sq) >> 8);
      }
      good_mask &= ~pos->bb[1 ^ pos->side_2_move];
    }
    good_mask &= ~pos->bb[pos->side_2_move];
    int next_sq;
    while (good_mask) {
      next_sq = bitScanForward(good_mask);
      if (((pos->bb[mPawn] >> sq) & 1) &&
          (((sq >> 3) == 6 && pos->side_2_move == mWhite) ||
           ((sq >> 3) == 1 && pos->side_2_move == mBlack))) {
        movelist_add_promotion(sq, next_sq, move_list, move_ptr, false);
      }
      if ((pos->bb[mPawn] >> sq) & 1 &&
          (next_sq - sq == 16 || sq - next_sq == 16)) {
        move_list[move_ptr++] = create_move(sq, next_sq, MOVEFLAG_PAWN_DPUSH);
      } else {
        move_list[move_ptr++] = create_move(
            sq, next_sq,
            (((pos->bb[1 ^ pos->side_2_move] >> next_sq) & 1) << 2));
      }

      good_mask ^= (1ULL << next_sq);
    }
    m ^= (1ULL << sq);
  }
}

void gen_pinned_pieces_moves(Position* pos, Move* move_list, U8& move_ptr) {
  gen_pinned_pieces_moves_diag(pos, move_list, move_ptr);
  gen_pinned_pieces_moves_nondiag(pos, move_list, move_ptr);
}

int gen_legal_moves(Position* pos, Move* move_list) {
  U8 move_ptr = 0;
  check_king_safety(pos);
  gen_king_moves(pos, pos->king_square[pos->side_2_move], move_list, move_ptr);
  gen_pinned_pieces_moves(pos, move_list, move_ptr);
  if (popCount(pos->attacks_on_king[pos->side_2_move]) < 2) {
    U64 mask = pos->bb[pos->side_2_move] & (~pos->pinned_mask);
    int sq = 0;
    while (mask) {
      sq = bitScanForward(mask);
      switch (pos->board[sq]) {
        case 'r':
        case 'R':
          gen_rook_moves(pos, sq, move_list, move_ptr);
          break;
        case 'b':
        case 'B':
          gen_bishop_moves(pos, sq, move_list, move_ptr);
          break;
        case 'q':
        case 'Q':
          gen_rook_moves(pos, sq, move_list, move_ptr);
          gen_bishop_moves(pos, sq, move_list, move_ptr);
          break;
        case 'p':
        case 'P':
          gen_pawn_moves(pos, sq, move_list, move_ptr);
          break;
        case 'n':
        case 'N':
          gen_knight_moves(pos, sq, move_list, move_ptr);
          break;
      };
      mask ^= (1ULL << sq);
    }
  }
  return move_ptr;
}

Move parse_move(Position* pos, const char* s) {
  assert(strlen(s) == 4 || strlen(s) == 5);
  assert(isalpha(s[0]) && isdigit(s[1]) && isalpha(s[2]) && isdigit(s[3]));
  U8 from = (s[0] - 'a') + (s[1] - '1') * 8;
  U8 to = (s[2] - 'a') + (s[3] - '1') * 8;
  if (board_to_bb(pos->board[from]) == mPawn) {
    if (to - from == 16 || from - to == 16)
      return create_move(from, to, MOVEFLAG_PAWN_DPUSH);
    if (to == pos->en_passant_sq[pos->ply])
      return create_move(from, to, MOVEFLAG_CAPTURE_EP);
    if (pos->side_2_move == mWhite && (to >> 3) == 7) {
      assert(strlen(s) == 5 && isalpha(s[4]));
      if (pos->board[to]) {
        if (s[4] == 'q')
          return create_move(from, to, MOVEFLAG_PROMOTE_QUEEN_CAPTURE);
        else if (s[4] == 'r')
          return create_move(from, to, MOVEFLAG_PROMOTE_ROOK_CAPTURE);
        else if (s[4] == 'b')
          return create_move(from, to, MOVEFLAG_PROMOTE_BISHOP_CAPTURE);
        else if (s[4] == 'n')
          return create_move(from, to, MOVEFLAG_PROMOTE_KNIGHT_CAPTURE);
      } else {
        if (s[4] == 'q')
          return create_move(from, to, MOVEFLAG_PROMOTE_QUEEN);
        else if (s[4] == 'r')
          return create_move(from, to, MOVEFLAG_PROMOTE_ROOK);
        else if (s[4] == 'b')
          return create_move(from, to, MOVEFLAG_PROMOTE_BISHOP);
        else if (s[4] == 'n')
          return create_move(from, to, MOVEFLAG_PROMOTE_KNIGHT);
      }
    }
  }
  if (board_to_bb(pos->board[from]) == mKing) {
    if (to == from + 2) return create_move(from, to, MOVEFLAG_CASTLE_KING);
    if (to == from - 2) return create_move(from, to, MOVEFLAG_CASTLE_QUEEN);
  }
  if (pos->board[to]) {
    return create_move(from, to, MOVEFLAG_CAPTURE);
  }
  return create_move(from, to, MOVEFLAG_QUIET);
}

char* move_as_str(Move m) {
  char* s;
  if (move_get_flags(m) >= MOVEFLAG_PROMOTE_KNIGHT) {
    s = (char*)malloc(5 * sizeof(char));
  } else {
    s = (char*)malloc(4 * sizeof(char));
  }
  s[0] = 'a' + (move_get_from(m) & 7);
  s[1] = '1' + (move_get_from(m) >> 3);
  s[2] = 'a' + (move_get_to(m) & 7);
  s[3] = '1' + (move_get_to(m) >> 3);
  switch (move_get_flags(m)) {
    case MOVEFLAG_PROMOTE_QUEEN:
    case MOVEFLAG_PROMOTE_QUEEN_CAPTURE:
      s[4] = 'q';
      break;
    case MOVEFLAG_PROMOTE_ROOK:
    case MOVEFLAG_PROMOTE_ROOK_CAPTURE:
      s[4] = 'r';
      break;
    case MOVEFLAG_PROMOTE_BISHOP:
    case MOVEFLAG_PROMOTE_BISHOP_CAPTURE:
      s[4] = 'b';
      break;
    case MOVEFLAG_PROMOTE_KNIGHT:
    case MOVEFLAG_PROMOTE_KNIGHT_CAPTURE:
      s[4] = 'n';
      break;
  }
  return s;
}
