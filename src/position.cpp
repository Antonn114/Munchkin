#include "position.h"
#include "bitboard.h"
#include "bittricks.h"
#include "defs.h"
Move create_move(U8 from, U8 to, U8 flags){
  return ((flags & 0xf)<<12) | ((from & 0x3f)<<6) | (to & 0x3f);
}

void
Position::gen_rook_moves(int sq){
  U64 good_mask = get_rook_attacks_raw(bitboard[1^side_2_move], sq);
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
  U64 good_mask = get_bishop_attacks_raw(bitboard[1^side_2_move], sq);
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
  // castling : king
  // castling : queen
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
  attacks_on_king |= knight_attacks[king_square[side_2_move]] & bitboard[1^side_2_move] & bitboard[mKnight];
  attacks_on_king |= rook_attacks[king_square[side_2_move]] & bitboard[1^side_2_move] & bitboard[mRook];
  attacks_on_king |= bishop_attacks[king_square[side_2_move]] & bitboard[1^side_2_move] & bitboard[mBishop];
  attacks_on_king |= queen_attacks[king_square[side_2_move]] & bitboard[1^side_2_move] & bitboard[mQueen];
  attacks_on_king |= king_attacks[king_square[side_2_move]] & bitboard[1^side_2_move] & bitboard[mKing];
  attacks_on_king |= pawn_white_attacks[king_square[side_2_move]] & bitboard[1^side_2_move] & bitboard[mPawn];

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
  U64 pinner_mask = 0;
  U64 mask = bitboard[1^side_2_move]; int sq = 0;
  U64 friendly_mask = bitboard[side_2_move] ^ (1ULL << king_square[side_2_move]);
  while(mask){
    sq = bitScanForward(mask);
    switch(mailbox[sq]){
      case 'r': case 'R': king_danger_squares |= get_rook_attacks_raw(friendly_mask, sq); 
                          pinner_mask |= get_rook_attacks_raw(bitboard[side_2_move], sq); break;
      case 'b': case 'B': king_danger_squares |= get_bishop_attacks_raw(friendly_mask, sq);
                          pinner_mask |= get_bishop_attacks_raw(bitboard[side_2_move], sq); break;
      case 'q': case 'Q': king_danger_squares |= get_queen_attacks_raw(friendly_mask, sq); 
                          pinner_mask |= get_queen_attacks_raw(bitboard[side_2_move], sq); break;
      case 'p': case 'P': king_danger_squares |= pawn_attacks[sq]; break;
      case 'n': case 'N': king_danger_squares |= knight_attacks[sq]; break;
      case 'k': case 'K': king_danger_squares |= king_attacks[sq]; break;
    } 
    mask ^= (1ULL << sq);
  }
  pinned_mask = pinner_mask & get_queen_attacks_raw(bitboard[1^side_2_move], king_square[side_2_move]);
}

void
Position::gen_legal_moves(){
  init_king_prerequisites();
  gen_king_moves(king_square[side_2_move]);
  gen_pinned_pieces_moves();
  if (popCount(attacks_on_king) > 1) return;
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
    }
    mask ^= (1ULL << sq);
  }
}
