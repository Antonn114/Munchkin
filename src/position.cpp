#include "position.h"

#include <iostream>
#include <sstream>

#include "bitboard.h"
#include "bittricks.h"
#include "defs.h"
#include "evaluation.h"

bool is_slider(Position* pos, int sq) {
  return ((pos->bb[mRook] | pos->bb[mBishop] | pos->bb[mQueen]) >> sq) & 1;
}

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

void check_king_safety(Position* pos) {
  U64 full_mask = pos->bb[mWhite] | pos->bb[mBlack];
  U64 full_mask_noking =
      full_mask ^ (1ULL << pos->king_square[pos->side_2_move]);
  pos->attacks_on_king[mBlack] = pos->attacks_on_king[mWhite] = 0;
  for (int i = 0; i < 2; i++) {
    pos->attacks_on_king[i] |= knight_attacks[pos->king_square[i]] &
                               (pos->bb[1 ^ i] & pos->bb[mKnight]);
    pos->attacks_on_king[i] |=
        get_rook_attacks(full_mask, pos->king_square[i]) &
        (pos->bb[1 ^ i] & (pos->bb[mRook] | pos->bb[mQueen]));
    pos->attacks_on_king[i] |=
        get_bishop_attacks(full_mask, pos->king_square[i]) &
        (pos->bb[1 ^ i] & (pos->bb[mBishop] | pos->bb[mQueen]));
    pos->attacks_on_king[i] |=
        king_attacks[pos->king_square[i]] & (pos->bb[1 ^ i] & pos->bb[mKing]);
    pos->attacks_on_king[i] |=
        pawn_attacks[pos->side_2_move][pos->king_square[i]] &
        (pos->bb[1 ^ i] & pos->bb[mPawn]);
  }

  pos->capture_mask = UNIVERSE;
  pos->push_mask = UNIVERSE;

  if (popCount(pos->attacks_on_king[pos->side_2_move]) == 1) {
    pos->capture_mask = pos->attacks_on_king[pos->side_2_move];
    int checker_sq = bitScanForward(pos->attacks_on_king[pos->side_2_move]);
    if (is_slider(pos, checker_sq)) {
      pos->push_mask = opponent_slider_to_king(
          checker_sq, pos->king_square[pos->side_2_move]);
    } else {
      pos->push_mask = 0;
    }
  }

  pos->king_danger_squares = 0;
  U64 mask = pos->bb[1 ^ pos->side_2_move];
  int sq = 0;
  pos->pinned_mask = 0;
  while (mask) {
    sq = bitScanForward(mask);
    switch (pos->board[sq]) {
      case 'r':
      case 'R':
        pos->king_danger_squares |= get_rook_attacks(full_mask_noking, sq);
        pos->pinned_mask |=
            ((get_rook_attacks(full_mask, sq) &
              get_rook_attacks(full_mask, pos->king_square[pos->side_2_move])) &
             opponent_slider_to_king(sq, pos->king_square[pos->side_2_move])) &
            pos->bb[pos->side_2_move];
        break;
      case 'b':
      case 'B':
        pos->king_danger_squares |= get_bishop_attacks(full_mask_noking, sq);
        pos->pinned_mask |=
            ((get_bishop_attacks(full_mask, sq) &
              get_bishop_attacks(full_mask,
                                 pos->king_square[pos->side_2_move])) &
             opponent_slider_to_king(sq, pos->king_square[pos->side_2_move])) &
            pos->bb[pos->side_2_move];
        break;
      case 'q':
      case 'Q':
        pos->king_danger_squares |= get_queen_attacks(full_mask_noking, sq);
        pos->pinned_mask |=
            ((get_queen_attacks(full_mask, sq) &
              get_queen_attacks(full_mask,
                                pos->king_square[pos->side_2_move])) &
             opponent_slider_to_king(sq, pos->king_square[pos->side_2_move])) &
            pos->bb[pos->side_2_move];
        break;
      case 'p':
      case 'P':
        pos->king_danger_squares |= pawn_attacks[1 ^ pos->side_2_move][sq];
        break;
      case 'n':
      case 'N':
        pos->king_danger_squares |= knight_attacks[sq];
        break;
      case 'k':
      case 'K':
        pos->king_danger_squares |= king_attacks[sq];
        break;
    };
    mask ^= (1ULL << sq);
  }
}

void parse_FEN_board(Position* pos, const std::string& fen) {
  int cnt = 0;
  for (U8 i = 0; i < 64; i++) {
    pos->board[i] = 0;
  }
  for (U8 i = 0; i < 8; i++) {
    pos->bb[i] = 0;
    pos->pst_score[i] = pos->pst_score[i + 8] = 0;
  }
  for (int c_i = 0; c_i < (int)fen.length() && cnt < 64; c_i++) {
    if (isdigit(fen[c_i])) {
      cnt += fen[c_i] - '0';
    } else if (fen[c_i] == '/') {
    } else {
      int sq = flipped_square(cnt);
      if (fen[c_i] == 'k') pos->king_square[mBlack] = sq;
      if (fen[c_i] == 'K') pos->king_square[mWhite] = sq;
      pos->board[sq] = fen[c_i];
      bool is_black = islower(fen[c_i]);
      pos->bb[is_black ? mBlack : mWhite] |= 1ULL << sq;
      int pst_lookup = is_black ? sq : cnt;
      int piece = board_to_bb(fen[c_i]);
      if (piece != mKing)
        pos->pst_score[piece + is_black * 8] +=
            piece_square_table[piece][pst_lookup];
      else
        pos->pst_score[piece + is_black * 8] +=
            is_endgame(pos) ? pst_King_Black_endgame[pst_lookup]
                            : pst_King_Black_middlegame[pst_lookup];
      pos->bb[board_to_bb(fen[c_i])] |= 1ULL << sq;
      cnt++;
    }
  }
}

void parse_FEN(Position* pos, const std::string& fen) {
  std::istringstream iss(fen);
  pos->ply = 0;
  std::string foo;
  iss >> foo;
  parse_FEN_board(pos, foo);
  iss >> foo;
  pos->side_2_move = (foo[0] == 'w' ? mWhite : mBlack);
  iss >> foo;
  if (foo[0] == '-') {
    pos->castling_rights[pos->ply] = 0;
  } else {
    for (char& c : foo) {
      if (c == 'K')
        pos->castling_rights[pos->ply] |= CASTLING_RIGHTS_WHITE_KING;
      else if (c == 'k')
        pos->castling_rights[pos->ply] |= CASTLING_RIGHTS_BLACK_KING;
      else if (c == 'Q')
        pos->castling_rights[pos->ply] |= CASTLING_RIGHTS_WHITE_QUEEN;
      else if (c == 'q')
        pos->castling_rights[pos->ply] |= CASTLING_RIGHTS_BLACK_QUEEN;
    }
  }
  iss >> foo;
  if (foo[0] == '-') {
    pos->en_passant_sq[pos->ply] = 0;

  } else {
    pos->en_passant_sq[pos->ply] = (foo[0] - 'a') * 8 + foo[0 + 1] - '0';
  }
  iss >> foo;
  pos->halfmove_clock[pos->ply] = stoi(foo);
  iss >> foo;
  pos->fullmove = stoi(foo);
  pos->ply++;
  pos->halfmove_clock[pos->ply] = pos->halfmove_clock[pos->ply - 1] + 1;
  pos->en_passant_sq[pos->ply] = pos->en_passant_sq[pos->ply - 1];
  pos->castling_rights[pos->ply] = pos->castling_rights[pos->ply - 1];
}

void print_board(Position* pos) {
  for (int i = 0; i < 64; i++) {
    U8 r = i >> 3;
    U8 c = i & 7;
    if (!isalpha(pos->board[(7 - r) * 8 + c])) {
      printf(".");
    } else {
      printf("%c", pos->board[(7 - r) * 8 + c]);
    }
    if ((i & 7) == 7) {
      printf("\n");
    }
  }
}

void print_bitboard_all(Position* pos) {
  printf("-- MASKS INFO --\n");
  printf("M_WHITE: 0x%llx\n", (U64)pos->bb[mWhite]);
  printf("M_BLACK: 0x%llx\n", (U64)pos->bb[mBlack]);
  printf("M_PAWN: 0x%llx\n", (U64)pos->bb[mPawn]);
  printf("M_KNIGHT: 0x%llx\n", (U64)pos->bb[mKnight]);
  printf("M_BISHOP: 0x%llx\n", (U64)pos->bb[mBishop]);
  printf("M_ROOK: 0x%llx\n", (U64)pos->bb[mRook]);
  printf("M_QUEEN: 0x%llx\n", (U64)pos->bb[mQueen]);
  printf("PINNED MASK: 0x%llx\n", (U64)pos->pinned_mask);
  printf("CAPTURE MASK: 0x%llx\n", (U64)pos->capture_mask);
  printf("PUSH MASK: 0x%llx\n", (U64)pos->push_mask);
  printf("KING DANGER: 0x%llx\n", (U64)pos->king_danger_squares);
  printf("ATK ON W KING: 0x%llx\n", (U64)pos->attacks_on_king[mWhite]);
  printf("ATK ON B KING: 0x%llx\n", (U64)pos->attacks_on_king[mBlack]);
}

bool is_opening(Position* pos) { return !is_middle(pos) && !is_endgame(pos); }
bool is_middle(Position* pos) {
  return std::max(
             popCount((pos->bb[mKnight] | pos->bb[mBishop]) & pos->bb[mWhite]),
             popCount((pos->bb[mKnight] | pos->bb[mBishop]) &
                      pos->bb[mBlack])) <= 3;
}
bool is_endgame(Position* pos) {
  return std::max(popCount((pos->bb[mKnight] | pos->bb[mBishop] |
                            pos->bb[mRook] | pos->bb[mQueen]) &
                           pos->bb[mWhite]),
                  popCount((pos->bb[mKnight] | pos->bb[mBishop] |
                            pos->bb[mRook] | pos->bb[mQueen]) &
                           pos->bb[mBlack])) <= 3;
}
