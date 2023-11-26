#ifndef __MUNCHKIN_SEARCH_H__
#define __MUNCHKIN_SEARCH_H__
#include "movegen.h"
#include "position.h"

int search(Position* pos, int depth, int alpha, int beta);
Move get_best_move(Position* pos, int depth);

// i would use int32/int64 buut i wanna refactorize it later
// i'll implement it later :3

#endif  // !__MUNCHKIN_SEARCH_H__
