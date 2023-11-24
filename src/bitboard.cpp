#include "bitboard.h"

#include <algorithm>

#include "bittricks.h"
// Attacks on empty board

U64 nort_attacks[64], sout_attacks[64], east_attacks[64], west_attacks[64];
U64 nowe_attacks[64], noea_attacks[64], sowe_attacks[64], soea_attacks[64];

U64 rank_attacks[64], file_attacks[64], diag_attacks[64], antidiag_attacks[64];

U64 rook_attacks[64], bishop_attacks[64], queen_attacks[64];

void init_attack_rays_positive() {
  U64 nort = C64(0x0101010101010100);
  for (int sq = 0; sq < 64; sq++, nort <<= 1) {
    nort_attacks[sq] = nort;
  }
  U64 noea = C64(0x8040201008040200);
  for (int f = 0; f < 8; f++, noea = eastOne(noea)) {
    U64 ne = noea;
    for (int r8 = 0; r8 < 64; r8 += 8, ne <<= 8) noea_attacks[r8 + f] = ne;
  }
  U64 nowe = C64(0x102040810204000);
  for (int f = 7; f >= 0; f--, nowe = westOne(nowe)) {
    U64 nw = nowe;
    for (int r8 = 0; r8 < 64; r8 += 8, nw <<= 8) {
      nowe_attacks[r8 + f] = nw;
    }
  }
  U64 east = C64(0xfe);
  for (int f = 0; f < 8; f++, east = eastOne(east)) {
    U64 ea = east;
    for (int r8 = 0; r8 < 64; r8 += 8, ea <<= 8) east_attacks[r8 + f] = ea;
  }
}

void init_attack_rays_negative() {
  U64 sout = C64(0x0080808080808080);
  for (int sq = 63; sq >= 0; sq--, sout >>= 1) sout_attacks[sq] = sout;

  U64 sowe = C64(0x40201008040201);
  for (int f = 7; f >= 0; f--, sowe = westOne(sowe)) {
    U64 sw = sowe;
    for (int r8 = 56; r8 >= 0; r8 -= 8, sw >>= 8) sowe_attacks[r8 + f] = sw;
  }
  U64 soea = C64(0x2040810204080);
  for (int f = 0; f < 8; f++, soea = eastOne(soea)) {
    U64 se = soea;
    for (int r8 = 56; r8 >= 0; r8 -= 8, se >>= 8) soea_attacks[r8 + f] = se;
  }
  U64 west = C64(0x7f00000000000000);
  for (int f = 7; f >= 0; f--, west = westOne(west)) {
    U64 we = west;
    for (int r8 = 56; r8 >= 0; r8 -= 8, we >>= 8) west_attacks[r8 + f] = we;
  }
}

void init_attack_rays() {
  init_attack_rays_positive();
  init_attack_rays_negative();
}
void init_attack_lines() {
  for (int sq = 0; sq < 64; sq++) {
    rank_attacks[sq] = east_attacks[sq] | west_attacks[sq];
    file_attacks[sq] = nort_attacks[sq] | sout_attacks[sq];
    diag_attacks[sq] = noea_attacks[sq] | sowe_attacks[sq];
    antidiag_attacks[sq] = nowe_attacks[sq] | soea_attacks[sq];
  }
}
void init_attack_pieces() {
  for (int sq = 0; sq < 64; sq++) {
    rook_attacks[sq] = rank_attacks[sq] | file_attacks[sq];
    bishop_attacks[sq] = diag_attacks[sq] | antidiag_attacks[sq];
    queen_attacks[sq] = rook_attacks[sq] | bishop_attacks[sq];
  }
  init_other_attacks();
}

void init_attack_masks() {
  init_attack_rays();
  init_attack_lines();
  init_attack_pieces();
}

// Attacks for real

U64** bishop_attacks_legal;
U64** rook_attacks_legal;

struct SMagic {
  U64 mask;   // to mask relevant squares of both lines (no outer squares)
  U64 magic;  // magic 64-bit factor
};

SMagic bishop_magic[64];
SMagic rook_magic[64];

U64 get_rook_attacks(U64 occ, enumSquare sq) {
  occ &= rook_magic[sq].mask;
  occ *= rook_magic[sq].magic;
  occ >>= 64 - RBits[sq];
  return rook_attacks_legal[sq][occ];
}

U64 get_bishop_attacks(U64 occ, enumSquare sq) {
  occ &= bishop_magic[sq].mask;
  occ *= bishop_magic[sq].magic;
  occ >>= 64 - BBits[sq];
  return bishop_attacks_legal[sq][occ];
}

U64 get_queen_attacks(U64 occ, enumSquare sq) {
  return get_rook_attacks(occ, sq) | get_bishop_attacks(occ, sq);
}

U64 bishop_good_masks_get(U64 blocker_mask, int sq) {
  U64 good_mask = 0;
  int isq;
  int cnt;

  for (isq = sq + 7, cnt = 0; cnt < std::min(sq & 7, 7 - (sq >> 3));
       cnt++, isq += 7) {
    good_mask |= (1ULL << isq);
    if ((blocker_mask >> isq) & 1) break;
  }
  for (isq = sq + 9, cnt = 0; cnt < std::min(7 - (sq & 7), 7 - (sq >> 3));
       cnt++, isq += 9) {
    good_mask |= (1ULL << isq);
    if ((blocker_mask >> isq) & 1) break;
  }
  for (isq = sq - 7, cnt = 0; cnt < std::min(7 - (sq & 7), (sq >> 3));
       cnt++, isq -= 7) {
    good_mask |= (1ULL << isq);
    if ((blocker_mask >> isq) & 1) break;
  }
  for (isq = sq - 9, cnt = 0; cnt < std::min((sq & 7), (sq >> 3));
       cnt++, isq -= 9) {
    good_mask |= (1ULL << isq);
    if ((blocker_mask >> isq) & 1) break;
  }
  return good_mask;
}

void init_bishop_legal_moves() {
  bishop_attacks_legal = (U64**)malloc(64 * sizeof(U64*));
  for (int sq = 0; sq < 64; sq++) {
    bishop_attacks_legal[sq] =
        (U64*)malloc(sizeof(U64) * (1 << popCount(bishop_magic[sq].mask)));
    for (int i = 0; i < (1 << popCount(bishop_magic[sq].mask)); i++) {
      U64 temp = 0;
      int ptr = 0;
      for (int j = 0; j < 64; j++) {
        if ((bishop_magic[sq].mask >> j) & 1) {
          temp |= ((i >> ptr) & 1) * (1ULL << j);
          ptr++;
        }
      }
      U64 good_mask = bishop_good_masks_get(temp, sq);
      temp *= bishop_magic[sq].magic;
      temp >>= 64 - BBits[sq];
      bishop_attacks_legal[sq][temp] = good_mask;
    }
  }
}

U64 rook_good_masks_get(U64 blocker_mask, int sq) {
  U64 good_mask = 0;
  int isq;

  for (isq = sq + 8; isq < 64; isq += 8) {
    good_mask |= (1ULL << isq);
    if ((blocker_mask >> isq) & 1) break;
  }
  for (isq = sq + 1; (isq >> 3) == (sq >> 3); isq += 1) {
    good_mask |= (1ULL << isq);
    if ((blocker_mask >> isq) & 1) break;
  }
  for (isq = sq - 8; isq >= 0; isq -= 8) {
    good_mask |= (1ULL << isq);
    if ((blocker_mask >> isq) & 1) break;
  }
  for (isq = sq - 1; (isq >> 3) == (sq >> 3); isq -= 1) {
    good_mask |= (1ULL << isq);
    if ((blocker_mask >> isq) & 1) break;
  }
  return good_mask;
}

void init_rook_legal_moves() {
  rook_attacks_legal = (U64**)malloc(64 * sizeof(U64*));
  for (int sq = 0; sq < 64; sq++) {
    rook_attacks_legal[sq] =
        (U64*)malloc(sizeof(U64) * (1 << popCount(rook_magic[sq].mask)));
    for (int i = 0; i < (1 << popCount(rook_magic[sq].mask)); i++) {
      U64 temp = 0;
      int ptr = 0;
      for (int j = 0; j < 64; j++) {
        if ((rook_magic[sq].mask >> j) & 1) {
          temp |= ((i >> ptr) & 1) * (1ULL << j);
          ptr++;
        }
      }
      U64 good_mask = rook_good_masks_get(temp, sq);
      temp *= rook_magic[sq].magic;
      temp >>= 64 - RBits[sq];
      rook_attacks_legal[sq][temp] = good_mask;
    }
  }
}

void init_magic() {
  for (int i = 0; i < 64; i++) {
    bishop_magic[i].magic = BMagic[i];
    rook_magic[i].magic = RMagic[i];
    bishop_magic[i].mask =
        bishop_attacks[i] & (UNIVERSE ^ (A_FILE | H_FILE | F_RANK | E_RANK));
    rook_magic[i].mask =
        rook_attacks[i] &
        (UNIVERSE ^ (A_FILE * ((i & 7) != 0) | H_FILE * ((i & 7) != 7) |
                     F_RANK * ((i >> 3) != 0) | E_RANK * ((i >> 3) != 8)));
  }
  init_bishop_legal_moves();
  init_rook_legal_moves();
}

U64 knight_attacks[64], king_attacks[64], pawn_attacks[2][64];

void init_other_attacks() {
  for (int i = 0; i < 64; i++) {
    U64 temp = (1ULL << i);
    knight_attacks[i] = nortOne(noEaOne(temp)) | noEaOne(eastOne(temp)) |
                        soEaOne(eastOne(temp)) | soutOne(soEaOne(temp)) |
                        nortOne(noWeOne(temp)) | noWeOne(westOne(temp)) |
                        soWeOne(westOne(temp)) | soutOne(soWeOne(temp));
    king_attacks[i] = noEaOne(temp) | nortOne(temp) | noWeOne(temp) |
                      westOne(temp) | soWeOne(temp) | soutOne(temp) |
                      soEaOne(temp) | eastOne(temp);
    pawn_attacks[0][i] = noEaOne(temp) | noWeOne(temp);
    pawn_attacks[1][i] = soEaOne(temp) | soWeOne(temp);
  }
}

U64 opponent_slider_to_king(int Esq, int Ksq) {
  return (nort_attacks[Esq] & sout_attacks[Ksq]) |
         (sout_attacks[Esq] & nort_attacks[Ksq]) |
         (east_attacks[Esq] & west_attacks[Ksq]) |
         (west_attacks[Esq] & east_attacks[Ksq]) |
         (noea_attacks[Esq] & sowe_attacks[Ksq]) |
         (nowe_attacks[Esq] & soea_attacks[Ksq]) |
         (soea_attacks[Esq] & nowe_attacks[Ksq]) |
         (sowe_attacks[Esq] & noea_attacks[Ksq]);
}
