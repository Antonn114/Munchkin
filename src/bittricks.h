#ifndef __MUNCHKIN_BITTRICKS_H__
#define __MUNCHKIN_BITTRICKS_H__
#include <assert.h>
#include "bitboard.h"
#include "defs.h"

U64 soutOne(U64 b), nortOne(U64 b), eastOne (U64 b), noEaOne(U64 b), soEaOne(U64 b), westOne(U64 b), soWeOne(U64 b), noWeOne(U64 b);

U64 noNoEa(U64 b), noEaEa(U64 b), soEaEa(U64 b), soSoEa(U64 b), noNoWe(U64 b), noWeWe(U64 b), soWeWe(U64 b), soSoWe(U64 b);

int popCount (U64 x); 

const int index64[64] = {
    0, 47,  1, 56, 48, 27,  2, 60,
   57, 49, 41, 37, 28, 16,  3, 61,
   54, 58, 35, 52, 50, 42, 21, 44,
   38, 32, 29, 23, 17, 11,  4, 62,
   46, 55, 26, 59, 40, 36, 15, 53,
   34, 51, 20, 43, 31, 22, 10, 45,
   25, 39, 14, 33, 19, 30,  9, 24,
   13, 18,  8, 12,  7,  6,  5, 63
};

int bitScanForward(U64 bb);


#endif // !__MUNCHKIN_BITTRICKS_H__
