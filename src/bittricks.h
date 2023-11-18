#ifndef __MUNCHKIN_BITTRICKS_H__
#define __MUNCHKIN_BITTRICKS_H__

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


#endif // !__MUNCHKIN_BITTRICKS_H__
