#ifndef __MUNCHKIN_MOVEUPD_H__
#define __MUNCHKIN_MOVEUPD_H__

#include "movegen.h"
#include "position.h"

void do_move(Position* pos, Move m);
void undo_move(Position* pos, Move m);

#endif  // !__MUNCHKIN_MOVEUPD_H__
