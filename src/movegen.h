#ifndef __MUNCHKIN_MOVEGEN_H__
#define __MUNCHKIN_MOVEGEN_H__

#include "defs.h"
#include "position.h"
typedef unsigned int Move;

#define MOVEFLAG_QUIET 0
#define MOVEFLAG_PAWN_DPUSH 1
#define MOVEFLAG_CASTLE_KING 2
#define MOVEFLAG_CASTLE_QUEEN 3
#define MOVEFLAG_CAPTURE 4
#define MOVEFLAG_CAPTURE_EP 5
#define MOVEFLAG_PROMOTE_KNIGHT 6
#define MOVEFLAG_PROMOTE_BISHOP 7
#define MOVEFLAG_PROMOTE_ROOK 8
#define MOVEFLAG_PROMOTE_QUEEN 9
#define MOVEFLAG_PROMOTE_KNIGHT_CAPTURE 10
#define MOVEFLAG_PROMOTE_BISHOP_CAPTURE 11
#define MOVEFLAG_PROMOTE_ROOK_CAPTURE 12
#define MOVEFLAG_PROMOTE_QUEEN_CAPTURE 13

extern U64 pinned_mask;
extern U64 capture_mask;
extern U64 push_mask;
extern U64 king_danger_squares;
extern U64 attacks_on_king;

Move create_move(U8 from, U8 to, U8 flags);
U8 move_get_flags(Move m);
U8 move_get_to(Move m);
U8 move_get_from(Move m);

void check_king_safety(Position *pos);

void gen_rook_moves(Position *pos, int sq, Move *move_list, U8 &move_ptr);
void gen_bishop_moves(Position *pos, int sq, Move *move_list, U8 &move_ptr);
void gen_knight_moves(Position *pos, int sq, Move *move_list, U8 &move_ptr);
void gen_pawn_moves(Position *pos, int sq, Move *move_list, U8 &move_ptr);
void gen_king_moves(Position *pos, int sq, Move *move_list, U8 &move_ptr);
void gen_pinned_pieces_moves(Position *pos, Move *move_list, U8 &move_ptr);
int gen_legal_moves(Position *pos, Move *move_list);

#endif  // !__MUNCHKIN_MOVEGEN_H__
