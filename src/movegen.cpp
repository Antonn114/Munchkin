#include "movegen.h"

#include "bitboard.h"
#include "bittricks.h"

U64 pinner_mask;
U64 pinned_mask;
U64 capture_mask;
U64 push_mask;
U64 king_danger_squares;
U64 attacks_on_king;

bool is_slider(Position* pos, int sq) {
  return ((pos->bb[mRook] | pos->bb[mBishop] | pos->bb[mQueen]) >> sq) & 1;
}

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
      pos->bb[1 ^ pos->side_2_move] | pos->bb[pos->side_2_move],
      (enumSquare)sq);
  good_mask &= ~pinned_mask;

  good_mask &= ~pos->bb[pos->side_2_move];
  good_mask &= capture_mask | push_mask;
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
      pos->bb[1 ^ pos->side_2_move] | pos->bb[pos->side_2_move],
      (enumSquare)sq);
  good_mask &= ~pinned_mask;
  good_mask &= ~pos->bb[pos->side_2_move];
  good_mask &= capture_mask | push_mask;
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
  good_mask &= ~pinned_mask;
  good_mask &= ~pos->bb[pos->side_2_move];
  good_mask &= capture_mask | push_mask;
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
  good_mask &= ~pinned_mask;
  good_mask &= ~pos->bb[pos->side_2_move];
  good_mask &= pos->bb[1 ^ pos->side_2_move];
  good_mask &= capture_mask | push_mask;
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
  good_mask = pawn_attacks[pos->side_2_move][sq];
  good_mask &= ~pinned_mask;

  good_mask &= pos->bb[1 ^ pos->side_2_move];
  good_mask &= capture_mask | push_mask;
  good_mask &= (1ULL << pos->en_passant_sq[pos->ply]);
  if (good_mask) {
    // checking that one position
    U64 company =
        (1ULL << sq) | (1ULL << (pos->en_passant_sq[pos->ply] +
                                 (pos->side_2_move == mWhite ? -8 : 8)));
    U64 funny = get_rook_attacks(
        (pos->bb[1 ^ pos->side_2_move] | pos->bb[pos->side_2_move]) ^ company,
        (enumSquare)pos->king_square[pos->side_2_move]);
    funny &= west_attacks[pos->king_square[pos->side_2_move]] |
             east_attacks[pos->king_square[pos->side_2_move]];
    funny &= ~pos->bb[pos->side_2_move];
    if ((funny & (pos->bb[mRook] | pos->bb[mQueen])) ^ 1) {
      move_list[move_ptr++] =
          create_move(sq, pos->en_passant_sq[pos->ply], MOVEFLAG_CAPTURE_EP);
    }
  }
}

void gen_pawn_push_moves(Position* pos, int sq, Move* move_list, U8& move_ptr) {
  U64 good_mask = (1ULL << sq);
  good_mask &= ~pinned_mask;
  if (pos->side_2_move == mWhite)
    good_mask <<= 8;
  else
    good_mask >>= 8;
  good_mask &= ~pos->bb[pos->side_2_move];
  good_mask &= ~pos->bb[1 ^ pos->side_2_move];
  bool cant_push = (good_mask == 0);
  good_mask &= push_mask;
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
  good_mask &= ~pinned_mask;
  if (pos->side_2_move == mWhite && (sq >> 3) == 1) {
    good_mask <<= 16;
  } else if (pos->side_2_move == mBlack && (sq >> 3) == 6) {
    good_mask >>= 16;
  } else
    return;
  good_mask &= ~pos->bb[pos->side_2_move];
  good_mask &= ~pos->bb[1 ^ pos->side_2_move];
  good_mask &= push_mask;
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
  good_mask &= ~king_danger_squares;
  good_mask &= capture_mask | push_mask;

  int next_sq;
  while (good_mask) {
    next_sq = bitScanForward(good_mask);
    move_list[move_ptr++] = create_move(
        sq, next_sq,
        ((pos->bb[1 ^ pos->side_2_move] >> next_sq) & 1) * MOVEFLAG_CAPTURE);
    good_mask ^= (1ULL << next_sq);
  }
  if (popCount(attacks_on_king) > 0) return;
  // castling : king
  if (((pos->side_2_move == mWhite &&
        (pos->castling_rights[pos->ply] & CASTLING_RIGHTS_WHITE_KING)) ||
       (pos->side_2_move == mBlack &&
        (pos->castling_rights[pos->ply] & CASTLING_RIGHTS_BLACK_KING))) &&
      (pos->board[sq + 1] == 0 && pos->board[sq + 2] == 0)) {
    move_list[move_ptr++] = create_move(sq, sq + 2, MOVEFLAG_CASTLE_KING);
  }
  // castling : queen
  if (((pos->side_2_move == mWhite &&
        (pos->castling_rights[pos->ply] & CASTLING_RIGHTS_WHITE_QUEEN)) ||
       (pos->side_2_move == mBlack &&
        (pos->castling_rights[pos->ply] & CASTLING_RIGHTS_BLACK_QUEEN))) &&
      (pos->board[sq - 1] == 0 && pos->board[sq - 2] == 0 &&
       pos->board[sq - 3] == 0)) {
    move_list[move_ptr++] = create_move(sq, sq - 2, MOVEFLAG_CASTLE_QUEEN);
  }
}

void gen_pinned_pieces_moves_diag(Position* pos, Move* move_list,
                                  U8& move_ptr) {
  U64 m = pinned_mask & (pos->bb[mBishop] | pos->bb[mQueen] | pos->bb[mPawn]);

  U8 sq;
  while (m) {
    sq = bitScanForward(m);
    U64 good_mask = get_bishop_attacks(
        pos->bb[pos->side_2_move] | pos->bb[1 ^ pos->side_2_move],
        (enumSquare)sq);
    if (((noea_attacks[sq] | sowe_attacks[sq]) >>
         pos->king_square[pos->side_2_move]) &
        1) {
      good_mask &= (noea_attacks[sq] | sowe_attacks[sq]);
    } else {
      good_mask &= (soea_attacks[sq] | nowe_attacks[sq]);
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
  U64 m = pinned_mask & (pos->bb[mRook] | pos->bb[mQueen] | pos->bb[mPawn]);
  U8 sq;
  while (m) {
    sq = bitScanForward(m);
    U64 good_mask = get_rook_attacks(
        pos->bb[pos->side_2_move] | pos->bb[1 ^ pos->side_2_move],
        (enumSquare)sq);
    if (((nort_attacks[sq] | sout_attacks[sq]) >>
         pos->king_square[pos->side_2_move]) &
        1) {
      good_mask &= (nort_attacks[sq] | sout_attacks[sq]);
    } else {
      good_mask &= (east_attacks[sq] | west_attacks[sq]);
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
      move_list[move_ptr++] = create_move(
          sq, next_sq, (((pos->bb[1 ^ pos->side_2_move] >> next_sq) & 1) << 2));
      good_mask ^= (1ULL << next_sq);
    }
    m ^= (1ULL << sq);
  }
}

void gen_pinned_pieces_moves(Position* pos, Move* move_list, U8& move_ptr) {
  gen_pinned_pieces_moves_diag(pos, move_list, move_ptr);
  gen_pinned_pieces_moves_nondiag(pos, move_list, move_ptr);
}

void check_king_safety(Position* pos) {
  attacks_on_king = 0;
  attacks_on_king |= knight_attacks[pos->king_square[pos->side_2_move]] &
                     (pos->bb[1 ^ pos->side_2_move] & pos->bb[mKnight]);
  attacks_on_king |= rook_attacks[pos->king_square[pos->side_2_move]] &
                     (pos->bb[1 ^ pos->side_2_move] & pos->bb[mRook]);
  attacks_on_king |= bishop_attacks[pos->king_square[pos->side_2_move]] &
                     (pos->bb[1 ^ pos->side_2_move] & pos->bb[mBishop]);
  attacks_on_king |= queen_attacks[pos->king_square[pos->side_2_move]] &
                     (pos->bb[1 ^ pos->side_2_move] & pos->bb[mQueen]);
  attacks_on_king |= king_attacks[pos->king_square[pos->side_2_move]] &
                     (pos->bb[1 ^ pos->side_2_move] & pos->bb[mKing]);
  attacks_on_king |=
      pawn_attacks[pos->side_2_move][pos->king_square[pos->side_2_move]] &
      (pos->bb[1 ^ pos->side_2_move] & pos->bb[mPawn]);

  capture_mask = UNIVERSE;
  push_mask = UNIVERSE;

  if (popCount(attacks_on_king) == 1) {
    capture_mask = attacks_on_king;
    int checker_sq = bitScanForward(attacks_on_king);
    if (is_slider(pos, checker_sq)) {
      push_mask = opponent_slider_to_king(checker_sq,
                                          pos->king_square[pos->side_2_move]);
    } else {
      push_mask = 0;
    }
  }

  king_danger_squares = 0;
  pinner_mask = 0;
  U64 mask = pos->bb[1 ^ pos->side_2_move];
  int sq = 0;
  U64 full_mask = pos->bb[mWhite] | pos->bb[mBlack];
  U64 full_mask_noking =
      full_mask ^ (1ULL << pos->king_square[1 ^ pos->side_2_move]);
  while (mask) {
    sq = bitScanForward(mask);
    switch (pos->board[sq]) {
      case 'r':
      case 'R':
        king_danger_squares |=
            get_rook_attacks(full_mask_noking, (enumSquare)sq);
        pinner_mask |= get_rook_attacks(full_mask, (enumSquare)sq);
        break;
      case 'b':
      case 'B':
        king_danger_squares |=
            get_bishop_attacks(full_mask_noking, (enumSquare)sq);
        pinner_mask |= get_bishop_attacks(full_mask, (enumSquare)sq);
        break;
      case 'q':
      case 'Q':
        king_danger_squares |=
            get_queen_attacks(full_mask_noking, (enumSquare)sq);
        pinner_mask |= get_queen_attacks(full_mask, (enumSquare)sq);
        break;
      case 'p':
      case 'P':
        king_danger_squares |= pawn_attacks[1 ^ pos->side_2_move][sq];
        break;
      case 'n':
      case 'N':
        king_danger_squares |= knight_attacks[sq];
        break;
      case 'k':
      case 'K':
        king_danger_squares |= king_attacks[sq];
        break;
    };
    mask ^= (1ULL << sq);
  }
  pinned_mask = pinner_mask &
                get_queen_attacks(
                    full_mask, (enumSquare)pos->king_square[pos->side_2_move]);
}

int gen_legal_moves(Position* pos, Move* move_list) {
  U8 move_ptr = 0;
  check_king_safety(pos);
  gen_king_moves(pos, pos->king_square[pos->side_2_move], move_list, move_ptr);
  gen_pinned_pieces_moves(pos, move_list, move_ptr);
  if (popCount(attacks_on_king) < 2) {
    U64 mask = pos->bb[pos->side_2_move];
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
