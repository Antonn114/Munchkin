#include "bittricks.h"

U64 soutOne (U64 b) { return b >> 8;}
U64 nortOne (U64 b) { return b << 8;}
U64 eastOne (U64 b) {return (b << 1) & (~A_FILE);}
U64 noEaOne (U64 b) {return (b << 9) & (~A_FILE);}
U64 soEaOne (U64 b) {return (b >> 7) & (~A_FILE);}
U64 westOne (U64 b) {return (b >> 1) & (~H_FILE);}
U64 soWeOne (U64 b) {return (b >> 9) & (~H_FILE);}
U64 noWeOne (U64 b) {return (b << 7) & (~H_FILE);}

int popCount (U64 x) {
   int count = 0;
   while (x) {
       count++;
       x &= x - 1; // reset LS1B
   }
   return count;
}
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
