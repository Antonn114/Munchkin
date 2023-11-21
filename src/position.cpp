#include "position.h"
#include "bitboard.h"
#include "bittricks.h"
#include "defs.h"
Move create_move(U8 from, U8 to, U8 flags){
  return ((flags & 0xf)<<12) | ((from & 0x3f)<<6) | (to & 0x3f);
}

U8 move_get_flags(Move m) {
  return (m >> 12) & 0x0f;
}

U8 move_get_from(Move m) {
  return (m >> 6) & 0x3f;
}

U8 move_get_to(Move m) {
  return m & 0x3f;
}

U8 mailbox_to_bitboard_piece (U8 c) {
  return (c == 'P' || c == 'p')*mPawn 
  | (c == 'N' || c == 'n')*mKnight 
  | (c == 'B' || c == 'b')*mBishop 
  | (c == 'R' || c == 'r')*mRook 
  | (c == 'Q' || c == 'q')*mQueen 
  | (c == 'K' || c == 'k')*mKing;
}

void
Position::gen_rook_moves(int sq){
  U64 good_mask = get_rook_attacks_raw(bitboard[1^side_2_move] | bitboard[side_2_move], (enumSquare)sq);
  good_mask &= ~pinned_mask;

  good_mask &= ~bitboard[side_2_move];
  good_mask &= capture_mask | push_mask;
  int next_sq;
  while(good_mask){
    next_sq = bitScanForward(good_mask);
    move_list[move_ptr++] = create_move(sq, next_sq, (((bitboard[1^side_2_move] >> next_sq)&1)<<2));
    good_mask ^= (1ULL << next_sq);
  }
}

void
Position::gen_bishop_moves(int sq){
  U64 good_mask = get_bishop_attacks_raw(bitboard[1^side_2_move] | bitboard[side_2_move], (enumSquare)sq);
  good_mask &= ~pinned_mask;
  good_mask &= ~bitboard[side_2_move];
  good_mask &= capture_mask | push_mask;
  int next_sq;
  while(good_mask){
    next_sq = bitScanForward(good_mask);
    move_list[move_ptr++] = create_move(sq, next_sq, (((bitboard[1^side_2_move] >> next_sq)&1)<<2));
    good_mask ^= (1ULL << next_sq);
  }
}

void
Position::gen_knight_moves(int sq){
  U64 good_mask = knight_attacks[sq];
  good_mask &= ~pinned_mask;
  good_mask &= ~bitboard[side_2_move];
  good_mask &= capture_mask | push_mask;
  int next_sq;
  while(good_mask){
    next_sq = bitScanForward(good_mask);
    move_list[move_ptr++] = create_move(sq, next_sq, (((bitboard[1^side_2_move] >> next_sq)&1)<<2));
    good_mask ^= (1ULL << next_sq);
  }
}

void
Position::gen_pawn_moves(int sq){
  // normal diag capture (no en passant)
  U64 good_mask = pawn_attacks[side_2_move][sq];
  good_mask &= ~pinned_mask;
  good_mask &= ~bitboard[side_2_move];
  good_mask &= bitboard[1^side_2_move];
  good_mask &= capture_mask | push_mask;
  int next_sq;
  while(good_mask){
    next_sq = bitScanForward(good_mask);
    if (((sq >> 3) == 6 && side_2_move == mWhite) || ((sq >> 3) == 1 && side_2_move == mBlack)){
      move_list[move_ptr++] = create_move(sq, next_sq, MOVEFLAG_PROMOTE_KNIGHT_CAPTURE);
      move_list[move_ptr++] = create_move(sq, next_sq, MOVEFLAG_PROMOTE_BISHOP_CAPTURE);
      move_list[move_ptr++] = create_move(sq, next_sq, MOVEFLAG_PROMOTE_ROOK_CAPTURE);
      move_list[move_ptr++] = create_move(sq, next_sq, MOVEFLAG_PROMOTE_QUEEN_CAPTURE);
    }
    else move_list[move_ptr++] = create_move(sq, next_sq, MOVEFLAG_CAPTURE);
    good_mask ^= (1ULL << next_sq);
  }
  // en passant
  good_mask = pawn_attacks[side_2_move][sq];
  good_mask &= ~pinned_mask;

  good_mask &= bitboard[1^side_2_move];
  good_mask &= capture_mask | push_mask;
  good_mask &= (1ULL << en_passant_sq[ply]);
  if (good_mask){
    // checking that one position
    U64 company = (1ULL << sq) | (1ULL << (en_passant_sq[ply] + (side_2_move == mWhite ? -8 : 8)));
    U64 funny = get_rook_attacks_raw((bitboard[1^side_2_move] | bitboard[side_2_move]) ^ company, (enumSquare)king_square[side_2_move]);
    funny &= west_attacks[king_square[side_2_move]] | east_attacks[king_square[side_2_move]];
    funny &= ~bitboard[side_2_move];
    if ((funny & (bitboard[mRook] | bitboard[mQueen])) ^ 1){
      move_list[move_ptr++] = create_move(sq, en_passant_sq[ply], MOVEFLAG_CAPTURE_EP);

    }
  }
  // push 
  good_mask = (1ULL << sq);
  good_mask &= ~pinned_mask;
  if (side_2_move == mWhite) good_mask <<= 8;
  else good_mask >>= 8;
  good_mask &= ~bitboard[side_2_move];
  good_mask &= ~bitboard[1^side_2_move];
  bool cant_push = (good_mask == 0);
  good_mask &= push_mask;
  if (good_mask){
    if (((sq >> 3) == 6 && side_2_move == mWhite) || ((sq >> 3) == 1 && side_2_move == mBlack)){
      move_list[move_ptr++] = create_move(sq, sq + (side_2_move == mWhite ? 8 : -8), MOVEFLAG_PROMOTE_ROOK);
      move_list[move_ptr++] = create_move(sq, sq + (side_2_move == mWhite ? 8 : -8), MOVEFLAG_PROMOTE_BISHOP);
      move_list[move_ptr++] = create_move(sq, sq + (side_2_move == mWhite ? 8 : -8), MOVEFLAG_PROMOTE_KNIGHT);
      move_list[move_ptr++] = create_move(sq, sq + (side_2_move == mWhite ? 8 : -8), MOVEFLAG_PROMOTE_QUEEN);

    }else move_list[move_ptr++] = create_move(sq, sq + (side_2_move == mWhite ? 8 : -8), MOVEFLAG_QUIET);
  }
  // double push
  if (cant_push) return;
  good_mask = (1ULL << sq);
  good_mask &= ~pinned_mask;
  if (side_2_move == mWhite && (sq >> 3) == 1){
    good_mask <<= 16;
  }else if (side_2_move == mBlack && (sq >> 3) == 6){
    good_mask >>= 16;
  }else return;
  good_mask &= ~bitboard[side_2_move];
  good_mask &= ~bitboard[1^side_2_move];
  good_mask &= push_mask;
  if (good_mask){
    move_list[move_ptr++] = create_move(sq, sq + (side_2_move == mBlack ? -16 : 16), MOVEFLAG_PAWN_DPUSH);
  } 
}

void
Position::gen_king_moves(int sq){
  // normal movement
  U64 good_mask = king_attacks[sq];
  good_mask &= ~bitboard[side_2_move];
  good_mask &= ~king_danger_squares;
  good_mask &= capture_mask | push_mask;

  int next_sq;
  while(good_mask){
    next_sq = bitScanForward(good_mask);
    move_list[move_ptr++] = create_move(sq, next_sq, (((bitboard[1^side_2_move] >> next_sq)&1)<<2));
    good_mask ^= (1ULL << next_sq);
  }
  if (popCount(attacks_on_king) > 0) return;
  // castling : king 
  if (((side_2_move == mWhite && (castling_rights[ply] & CASTLING_RIGHTS_WHITE_KING))
    || (side_2_move == mBlack && (castling_rights[ply] & CASTLING_RIGHTS_BLACK_KING)))
    && (mailbox[sq + 1] == 0 && mailbox[sq + 2] == 0)){
    move_list[move_ptr++] = create_move(sq, sq + 2, MOVEFLAG_CASTLE_KING);
  }
  // castling : queen
  if (((side_2_move == mWhite && (castling_rights[ply] & CASTLING_RIGHTS_WHITE_QUEEN))
    || (side_2_move == mBlack && (castling_rights[ply] & CASTLING_RIGHTS_BLACK_QUEEN)))
    && (mailbox[sq - 1] == 0 && mailbox[sq - 2] == 0 && mailbox[sq - 3] == 0)){
    move_list[move_ptr++] = create_move(sq, sq - 2, MOVEFLAG_CASTLE_QUEEN);
  }
}

bool 
Position::is_slider(int sq){
  return ((bitboard[mRook] | bitboard[mBishop] | bitboard[mQueen]) >> sq) & 1;
}

U64
Position::opponent_slider_to_king(int Esq, int Ksq){
  return (nort_attacks[Esq] & sout_attacks[Ksq])
  | (sout_attacks[Esq] & nort_attacks[Ksq])
  | (east_attacks[Esq] & west_attacks[Ksq])
  | (west_attacks[Esq] & east_attacks[Ksq])
  | (noea_attacks[Esq] & sowe_attacks[Ksq])
  | (nowe_attacks[Esq] & soea_attacks[Ksq])
  | (soea_attacks[Esq] & nowe_attacks[Ksq])
  | (sowe_attacks[Esq] & noea_attacks[Ksq]);
}


void
Position::init_king_prerequisites(){
  attacks_on_king = 0;
  attacks_on_king |= knight_attacks[king_square[side_2_move]] & (bitboard[1^side_2_move] & bitboard[mKnight]);
  attacks_on_king |= rook_attacks[king_square[side_2_move]] & (bitboard[1^side_2_move] & bitboard[mRook]);
  attacks_on_king |= bishop_attacks[king_square[side_2_move]] & (bitboard[1^side_2_move] & bitboard[mBishop]);
  attacks_on_king |= queen_attacks[king_square[side_2_move]] & (bitboard[1^side_2_move] & bitboard[mQueen]);
  attacks_on_king |= king_attacks[king_square[side_2_move]] & (bitboard[1^side_2_move] & bitboard[mKing]);
  attacks_on_king |= pawn_attacks[side_2_move][king_square[side_2_move]] & (bitboard[1^side_2_move] & bitboard[mPawn]);

  capture_mask = UNIVERSE;
  push_mask = UNIVERSE;

  if (popCount(attacks_on_king) == 1){
    capture_mask = attacks_on_king;
    int checker_sq = bitScanForward(attacks_on_king);
    if (is_slider(checker_sq)){
      push_mask = opponent_slider_to_king(checker_sq, king_square[side_2_move]);
    }else{
      push_mask = 0;
    }
  }

  king_danger_squares = 0;
  pinner_mask = 0;
  U64 mask = bitboard[1^side_2_move]; int sq = 0;
  U64 full_mask = bitboard[mWhite] | bitboard[mBlack];
  U64 full_mask_noking = full_mask ^ (1ULL << king_square[1^side_2_move]); 
  while(mask){
    sq = bitScanForward(mask);
    switch(mailbox[sq]){
      case 'r': case 'R': king_danger_squares |= get_rook_attacks_raw(full_mask_noking, (enumSquare)sq); 
        pinner_mask |= get_rook_attacks_raw(full_mask, (enumSquare)sq); break;
      case 'b': case 'B': king_danger_squares |= get_bishop_attacks_raw(full_mask_noking, (enumSquare)sq);
        pinner_mask |= get_bishop_attacks_raw(full_mask, (enumSquare)sq); break;
      case 'q': case 'Q': king_danger_squares |= get_queen_attacks_raw(full_mask_noking, (enumSquare)sq); 
        pinner_mask |= get_queen_attacks_raw(full_mask, (enumSquare)sq); break;
      case 'p': case 'P': king_danger_squares |= pawn_attacks[1^side_2_move][sq]; break;
      case 'n': case 'N': king_danger_squares |= knight_attacks[sq]; break;
      case 'k': case 'K': king_danger_squares |= king_attacks[sq]; break;
    };
    mask ^= (1ULL << sq);
  }
  pinned_mask = pinner_mask & get_queen_attacks_raw(full_mask, (enumSquare)king_square[side_2_move]);
}

void
Position::gen_pinned_pieces_moves(){
  // diag 
  U64 m = pinned_mask & (bitboard[mBishop] | bitboard[mQueen] | bitboard[mPawn]);

  int sq;
  while(m){
    sq = bitScanForward(m);
    U64 good_mask = get_bishop_attacks_raw(bitboard[side_2_move] | bitboard[1^side_2_move], (enumSquare)sq);
    if (((noea_attacks[sq] | sowe_attacks[sq]) >> king_square[side_2_move]) & 1){
      good_mask &= (noea_attacks[sq] | sowe_attacks[sq]);
    }else{
      good_mask &= (soea_attacks[sq] | nowe_attacks[sq]);
    }
    if ((bitboard[mPawn] >> sq)&1){
      good_mask &= pawn_attacks[side_2_move][sq];
      good_mask &= bitboard[1^side_2_move];
    }else{
      good_mask &= ~bitboard[side_2_move];
    }
    int next_sq;
    while(good_mask){
      next_sq = bitScanForward(good_mask);
      if (((bitboard[mPawn] >> sq)&1) && (
          ((sq >> 3) == 6 && side_2_move == mWhite) || ((sq >> 3) == 1 && side_2_move == mBlack))){
        move_list[move_ptr++] = create_move(sq, next_sq, MOVEFLAG_PROMOTE_KNIGHT_CAPTURE);
        move_list[move_ptr++] = create_move(sq, next_sq, MOVEFLAG_PROMOTE_BISHOP_CAPTURE);
        move_list[move_ptr++] = create_move(sq, next_sq, MOVEFLAG_PROMOTE_ROOK_CAPTURE);
        move_list[move_ptr++] = create_move(sq, next_sq, MOVEFLAG_PROMOTE_QUEEN_CAPTURE);
      }
      else move_list[move_ptr++] = create_move(sq, next_sq, (((bitboard[1^side_2_move] >> next_sq)&1)<<2));
      good_mask ^= (1ULL << next_sq);
    }
    m ^= (1ULL << sq);
  }

  // nondiag
  m = pinned_mask & (bitboard[mRook] | bitboard[mQueen] | bitboard[mPawn]);

  while(m){
    sq = bitScanForward(m);
    U64 good_mask = get_rook_attacks_raw(bitboard[side_2_move] | bitboard[1^side_2_move], (enumSquare)sq);
    if (((nort_attacks[sq] | sout_attacks[sq]) >> king_square[side_2_move]) & 1){
      good_mask &= (nort_attacks[sq] | sout_attacks[sq]);
    }else{
      good_mask &= (east_attacks[sq] | west_attacks[sq]);
    }
    if ((bitboard[mPawn] >> sq)&1){
      if (side_2_move == mWhite){
        good_mask &= nort_attacks[sq];
        if ((sq >> 3) == 1) good_mask &= C64(0xffffffff);
        else good_mask &= ((1ULL << sq) << 8);
      }
      else{
        good_mask &= sout_attacks[sq];
        if ((sq >> 3) == 6) good_mask &= C64(0xffffffff00000000);
        else good_mask &= ((1ULL << sq) >> 8);
      }
      good_mask &= ~bitboard[1^side_2_move];
    }
    good_mask &= ~bitboard[side_2_move];
    int next_sq;
    while(good_mask){
      next_sq = bitScanForward(good_mask);
      if (((bitboard[mPawn] >> sq)&1) && (
          ((sq >> 3) == 6 && side_2_move == mWhite) || ((sq >> 3) == 1 && side_2_move == mBlack))){
        move_list[move_ptr++] = create_move(sq, next_sq, MOVEFLAG_PROMOTE_KNIGHT);
        move_list[move_ptr++] = create_move(sq, next_sq, MOVEFLAG_PROMOTE_BISHOP);
        move_list[move_ptr++] = create_move(sq, next_sq, MOVEFLAG_PROMOTE_ROOK);
        move_list[move_ptr++] = create_move(sq, next_sq, MOVEFLAG_PROMOTE_QUEEN);
      }
      move_list[move_ptr++] = create_move(sq, next_sq, (((bitboard[1^side_2_move] >> next_sq)&1)<<2));
      good_mask ^= (1ULL << next_sq);
    }
    m ^= (1ULL << sq);
  }
}

int
Position::gen_legal_moves(Move ml[]){
  init_king_prerequisites();
  gen_king_moves(king_square[side_2_move]);
  printf("%llu %llu\n", pinner_mask, pinned_mask);
  gen_pinned_pieces_moves();
  if (popCount(attacks_on_king) < 2){
    U64 mask = bitboard[side_2_move];
    int sq = 0;
    while(mask){
      sq = bitScanForward(mask);
      switch(mailbox[sq]){
        case 'r': case 'R': gen_rook_moves(sq); break;
        case 'b': case 'B': gen_bishop_moves(sq); break;
        case 'q': case 'Q': gen_rook_moves(sq); gen_bishop_moves(sq); break;
        case 'p': case 'P': gen_pawn_moves(sq); break;
        case 'n': case 'N': gen_knight_moves(sq); break;
      };
      mask ^= (1ULL << sq);
    }
  }
  for (int i = 0; i < move_ptr; i++){
    ml[i] = move_list[i];
  }
  return move_ptr;
}


void
Position::do_move(Move m){
  U8 from, to, flags;
  from = move_get_from(m);
  to = move_get_to(m);
  flags = move_get_flags(m);
  switch(flags){
    case MOVEFLAG_QUIET:
      if (mailbox_to_bitboard_piece(mailbox[from]) == mPawn){
        halfmove_clock[ply] = 0;
      }
      break;
    case MOVEFLAG_PAWN_DPUSH:
      halfmove_clock[ply] = 0;
      en_passant_sq[ply] = enumSquare(to) + (side_2_move == mWhite ? -8 : 8);
      break;
    case MOVEFLAG_CASTLE_KING:
      mailbox[king_square[side_2_move] + 1] = mailbox[king_square[side_2_move] + 3];
      mailbox[king_square[side_2_move] + 3] = 0;
      bitboard[side_2_move] ^= (1ULL << (king_square[side_2_move] + 1)) | (1ULL << (king_square[side_2_move] + 3));
      bitboard[mRook] ^= (1ULL << (king_square[side_2_move] + 1)) | (1ULL << (king_square[side_2_move] + 3));      
      break;
    case MOVEFLAG_CASTLE_QUEEN:

      mailbox[king_square[side_2_move] - 1] = mailbox[king_square[side_2_move] - 4];
      mailbox[king_square[side_2_move] - 4] = 0;
      bitboard[side_2_move] ^= (1ULL << (king_square[side_2_move] - 1)) | (1ULL << (king_square[side_2_move] - 4));
      bitboard[mRook] ^= (1ULL << (king_square[side_2_move] - 1)) | (1ULL << (king_square[side_2_move] -4));      
      break;
    case MOVEFLAG_CAPTURE:
      halfmove_clock[ply] = 0;

      captured_history_list[captured_ptr++] = mailbox[to];
      bitboard[1^side_2_move] ^= (1ULL << (to));
      bitboard[mailbox_to_bitboard_piece(mailbox[to])] ^= (1ULL << (to));
      if (mailbox_to_bitboard_piece(mailbox[to]) == mRook && castling_rights[ply]){
        if (to == h1) castling_rights[ply] &= CASTLING_RIGHTS_WHITE_QUEEN | CASTLING_RIGHTS_BLACK_KING | CASTLING_RIGHTS_BLACK_QUEEN;
        if (to == a1) castling_rights[ply] &= CASTLING_RIGHTS_BLACK_QUEEN | CASTLING_RIGHTS_BLACK_KING | CASTLING_RIGHTS_WHITE_KING;
        if (to == h8) castling_rights[ply] &= CASTLING_RIGHTS_WHITE_KING | CASTLING_RIGHTS_BLACK_QUEEN | CASTLING_RIGHTS_WHITE_QUEEN;
        if (to == a8) castling_rights[ply] &= CASTLING_RIGHTS_WHITE_QUEEN | CASTLING_RIGHTS_WHITE_KING | CASTLING_RIGHTS_BLACK_KING;
      }
      break;
    case MOVEFLAG_CAPTURE_EP: 
      halfmove_clock[ply] = 0;
      captured_history_list[captured_ptr++] = mailbox[en_passant_sq[ply] + (side_2_move == mWhite ? -8 : 8)];
      mailbox[en_passant_sq[ply] + (side_2_move == mWhite ? -8 : 8)] = 0;
      bitboard[1^side_2_move] ^= (1ULL << (en_passant_sq[ply] + (side_2_move == mWhite ? -8 : 8)));
      bitboard[mPawn] ^= (1ULL << (en_passant_sq[ply] + (side_2_move == mWhite ? -8 : 8)));
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
      captured_history_list[captured_ptr++] = mailbox[to];
      break;
    case MOVEFLAG_PROMOTE_BISHOP_CAPTURE:
      halfmove_clock[ply] = 0;
      bitboard[mPawn] ^= (1ULL << from);
      bitboard[mBishop] ^= (1ULL << from);
      mailbox[from] += -'P' + 'B';
      captured_history_list[captured_ptr++] = mailbox[to];
      break;
    case MOVEFLAG_PROMOTE_ROOK_CAPTURE:
      halfmove_clock[ply] = 0;
      bitboard[mPawn] ^= (1ULL << from);
      bitboard[mRook] ^= (1ULL << from);
      mailbox[from] += -'P' + 'R';
      captured_history_list[captured_ptr++] = mailbox[to];
      break;
    case MOVEFLAG_PROMOTE_QUEEN_CAPTURE:
      halfmove_clock[ply] = 0;
      bitboard[mPawn] ^= (1ULL << from);
      bitboard[mQueen] ^= (1ULL << from);
      mailbox[from] += -'P' + 'Q';
      captured_history_list[captured_ptr++] = mailbox[to];
      break;
  };
  if (from == king_square[side_2_move] || flags == MOVEFLAG_CASTLE_KING || flags == MOVEFLAG_CASTLE_QUEEN){
    if (side_2_move == mWhite){
      castling_rights[ply] &= CASTLING_RIGHTS_BLACK_KING | CASTLING_RIGHTS_BLACK_QUEEN;
    }
    else {
      castling_rights[ply] &= CASTLING_RIGHTS_WHITE_KING | CASTLING_RIGHTS_WHITE_QUEEN;
    } 
  }
  if (mailbox_to_bitboard_piece(mailbox[from]) == mRook && castling_rights[ply]){
    if (from == h1) castling_rights[ply] &= CASTLING_RIGHTS_WHITE_QUEEN | CASTLING_RIGHTS_BLACK_KING | CASTLING_RIGHTS_BLACK_QUEEN;
    if (from == a1) castling_rights[ply] &= CASTLING_RIGHTS_BLACK_QUEEN | CASTLING_RIGHTS_BLACK_KING | CASTLING_RIGHTS_WHITE_KING;
    if (from == h8) castling_rights[ply] &= CASTLING_RIGHTS_WHITE_KING | CASTLING_RIGHTS_BLACK_QUEEN | CASTLING_RIGHTS_WHITE_QUEEN;
    if (from == a8) castling_rights[ply] &= CASTLING_RIGHTS_WHITE_QUEEN | CASTLING_RIGHTS_WHITE_KING | CASTLING_RIGHTS_BLACK_KING;
  }
  if (flags != MOVEFLAG_PAWN_DPUSH){
    en_passant_sq[ply] = 0;
  }
  bitboard[side_2_move] ^= (1ULL << to) | (1ULL << from);
  bitboard[mailbox_to_bitboard_piece(mailbox[from])] ^= (1ULL << to) | (1ULL << from);
  mailbox[to] = mailbox[from];
  mailbox[from] = 0;
  if (king_square[side_2_move] == from){
    king_square[side_2_move] = to;
  }
  en_passant_sq[ply + 1] = en_passant_sq[ply];
  castling_rights[ply + 1] = castling_rights[ply];
  halfmove_clock[ply + 1] = halfmove_clock[ply] + 1;
  ply++;
  side_2_move ^= 1;
  move_ptr = 0;
}

void
Position::undo_move(Move m){
  ply--;
  move_ptr = 0;
  side_2_move ^= 1;
  U8 from, to, flags;
  from = move_get_from(m);
  to = move_get_to(m);
  flags = move_get_flags(m);
  if (king_square[side_2_move] == to){
    king_square[side_2_move] = from;
  }
  mailbox[from] = mailbox[to];
  mailbox[to] = 0;
  bitboard[side_2_move] ^= (1ULL << to) | (1ULL << from);
  bitboard[mailbox_to_bitboard_piece(mailbox[from])] ^= (1ULL << to) | (1ULL << from);
  switch(flags){
    case MOVEFLAG_CASTLE_KING:
      mailbox[king_square[side_2_move] + 3] = mailbox[king_square[side_2_move] + 1];
      mailbox[king_square[side_2_move] + 1] = 0;

      bitboard[side_2_move] ^= (1ULL << (king_square[side_2_move] + 1)) | (1ULL << (king_square[side_2_move] + 3));
      bitboard[mRook] ^= (1ULL << (king_square[side_2_move] + 1)) | (1ULL << (king_square[side_2_move] + 3));      
      break;
    case MOVEFLAG_CASTLE_QUEEN:
      mailbox[king_square[side_2_move] - 4] = mailbox[king_square[side_2_move] - 1];
      mailbox[king_square[side_2_move] - 1] = 0;
      bitboard[side_2_move] ^= (1ULL << (king_square[side_2_move] - 1)) | (1ULL << (king_square[side_2_move] - 4));
      bitboard[mRook] ^= (1ULL << (king_square[side_2_move] - 1)) | (1ULL << (king_square[side_2_move] -4));      
      break;
    case MOVEFLAG_CAPTURE:
      captured_ptr--;
      mailbox[to] = captured_history_list[captured_ptr];
      bitboard[mailbox_to_bitboard_piece(captured_history_list[captured_ptr])] ^= (1ULL << (to));
      bitboard[1^side_2_move] ^= (1ULL << (to));
      break;
    case MOVEFLAG_CAPTURE_EP:
      mailbox[en_passant_sq[ply]] = 'P' + (side_2_move == mBlack)*('p' - 'P');
      captured_ptr--;
      bitboard[1^side_2_move] ^= (1ULL << (en_passant_sq[ply]));
      bitboard[mPawn] ^= (1ULL << (en_passant_sq[ply]));
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
      captured_ptr--;
      break;
    case MOVEFLAG_PROMOTE_BISHOP_CAPTURE:
      bitboard[mPawn] ^= (1ULL << from);
      bitboard[mBishop] ^= (1ULL << from);
      mailbox[from] -= -'P' + 'B';
      captured_ptr--;

      break;
    case MOVEFLAG_PROMOTE_ROOK_CAPTURE:
      bitboard[mPawn] ^= (1ULL << from);
      bitboard[mRook] ^= (1ULL << from);
      mailbox[from] -= -'P' + 'R';
      captured_ptr--;

      break;
    case MOVEFLAG_PROMOTE_QUEEN_CAPTURE:
      bitboard[mPawn] ^= (1ULL << from);
      bitboard[mQueen] ^= (1ULL << from);
      mailbox[from] -= -'P' + 'Q';
      captured_ptr--;

      break;
  };
}

void
Position::parse_FEN(const std::string& fen){
  int board_ptr = 56;
  for (int i = 0; i< 64; i++){
    mailbox[i] = 0;
  }
  bitboard[0] = bitboard[1] = bitboard[2] = bitboard[3] = bitboard[4] = bitboard[5] = bitboard[6] = bitboard[7] = 0;
  int c_i = 0;
  int cnt = 0;
  for (; c_i < (int)fen.length() && cnt < 64; c_i++){
    if (isspace(fen[c_i])) continue;
    else if (isdigit(fen[c_i])){
      board_ptr+=fen[c_i] - '0';
      cnt += fen[c_i] - '0';
    }else if (fen[c_i] == '/'){
      board_ptr -= 16;
    }else{
      if (fen[c_i] == 'k') king_square[mBlack] = board_ptr;
      if (fen[c_i] == 'K') king_square[mWhite] = board_ptr;
      mailbox[board_ptr] = fen[c_i];
      if (islower(fen[c_i])) bitboard[mBlack] |= (1ULL << board_ptr);
      else bitboard[mWhite] |= (1ULL << board_ptr);
      bitboard[mailbox_to_bitboard_piece(fen[c_i])] |= (1ULL << board_ptr);
      board_ptr++;
      cnt++;
    }
  }
  ply = 0;
  while(isspace(fen[c_i])) {c_i++;}
  if (fen[c_i] == 'w') side_2_move = mWhite;
  else side_2_move = mBlack;
  c_i++;
  while(isspace(fen[c_i])) {c_i++;}
  
  castling_rights[ply] = 0;

  if (fen[c_i] == '-') c_i++;
  else{
    while(!isspace(fen[c_i])){
      if (fen[c_i] == 'K') castling_rights[ply] |= CASTLING_RIGHTS_WHITE_KING;
      else if (fen[c_i] == 'k') castling_rights[ply] |= CASTLING_RIGHTS_BLACK_KING;
      else if (fen[c_i] == 'Q') castling_rights[ply] |= CASTLING_RIGHTS_WHITE_QUEEN;
      else if (fen[c_i] == 'q') castling_rights[ply] |= CASTLING_RIGHTS_BLACK_QUEEN;
      c_i++;
    }
    c_i++;
  }
  while(isspace(fen[c_i])) {c_i++;}
  en_passant_sq[ply] = 0;
  if (fen[c_i] == '-') c_i++;
  else {
    en_passant_sq[ply] = (fen[c_i] - 'a')*8 + fen[c_i + 1] - '0';
    c_i += 2;
  }
  while(isspace(fen[c_i])) {c_i++;}
  halfmove_clock[ply] = isspace(fen[c_i + 1]) ? fen[c_i] - '0' : (fen[c_i] - '0')*10 + fen[c_i + 1] - '0';

}

void
Position::display_mailbox(){
  printf("\n");
  int cnt = 0;
  for (int sq = 56; cnt < 64; cnt++){
    if (mailbox[sq] == 0) putchar(' ');
    else putchar(mailbox[sq]);
    sq++;
    if (sq % 8 == 0){
      putchar('\n');
      sq-=16;
    }
  }
  printf("\n");
}

void
Position::debug_bitboard(int c){
  printf("%llu\n", bitboard[c]);
}
