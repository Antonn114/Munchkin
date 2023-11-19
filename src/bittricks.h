#ifndef __MUNCHKIN_BITTRICKS_H__
#define __MUNCHKIN_BITTRICKS_H__
#include <assert.h>
#include "bitboard.h"
#include "defs.h"
U64 soutOne (U64 b) { return b >> 8;}
U64 nortOne (U64 b) { return b << 8;}
U64 eastOne (U64 b) {return (b << 1) & (~A_FILE);}
U64 noEaOne (U64 b) {return (b << 9) & (~A_FILE);}
U64 soEaOne (U64 b) {return (b >> 7) & (~A_FILE);}
U64 westOne (U64 b) {return (b >> 1) & (~H_FILE);}
U64 soWeOne (U64 b) {return (b >> 9) & (~H_FILE);}
U64 noWeOne (U64 b) {return (b << 7) & (~H_FILE);}

U64 noNoEa(U64 b) {return (b << 17) & (~A_FILE) ;}
U64 noEaEa(U64 b) {return (b << 10) & (~(A_FILE | (A_FILE >> 1)));}
U64 soEaEa(U64 b) {return (b >>  6) & (~(A_FILE | (A_FILE >> 1)));}
U64 soSoEa(U64 b) {return (b >> 15) & (~A_FILE);}
U64 noNoWe(U64 b) {return (b << 15) & (~H_FILE);}
U64 noWeWe(U64 b) {return (b <<  6) & (~(H_FILE | (H_FILE << 1)));}
U64 soWeWe(U64 b) {return (b >> 10) & (~(H_FILE | (H_FILE << 1)));}
U64 soSoWe(U64 b) {return (b >> 17) & (~H_FILE);}

int popCount (U64 x) {
   int count = 0;
   while (x) {
       count++;
       x &= x - 1; // reset LS1B
   }
   return count;
}

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

/**
 * bitScanForward
 * @author Kim Walisch (2012)
 * @param bb bitboard to scan
 * @precondition bb != 0
 * @return index (0..63) of least significant one bit
 */
int bitScanForward(U64 bb) {
   const U64 debruijn64 = C64(0x03f79d71b4cb0a89);
   assert (bb != 0);
   return index64[((bb ^ (bb-1)) * debruijn64) >> 58];
}


#endif // !__MUNCHKIN_BITTRICKS_H__
