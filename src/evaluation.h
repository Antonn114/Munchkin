#ifndef __MUNCHKIN_EVALUATION_H__
#define __MUNCHKIN_EVALUATION_H__
#include "movegen.h"
#include "position.h"

#define EVAL_WINNER_BLACK -1
#define EVAL_WINNER_WHITE 1
#define EVAL_STALEMATE 0
#define EVAL_NULL 10

int check_winner(Position* pos);
int eval(Position* pos);

#endif  // !__MUNCHKIN_EVALUATION_H__
