#ifndef __MUNCHKIN_POSITION_H__
#define __MUNCHKIN_POSITION_H__

#include "defs.h"
#include "bitboard.h"
#include "piece.h"

class Position{
private:
  Board bitboard;
  enumColor side_2_move;
  U8 castling_rights;
  enumSquare en_passant_sq;
  U32 halfmove_clock;
public:

};

#endif // !__MUNCHKIN_POSITION_H__
