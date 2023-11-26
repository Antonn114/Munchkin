#include <stdio.h>
#include <string.h>
#include <time.h>

#include "misc.h"
#include "movegen.h"
#include "munchkin.h"
#include "position.h"

Move move_history[6000];
U8 move_history_ptr = 0;
U8 cool_perft = 203;
U64 Perft(Position *pos, int depth) {
  Move move_list[256];
  int n_moves, i;
  U64 nodes = 0;
  if (depth == 0) return 1ULL;
  n_moves = gen_legal_moves(pos, move_list);
  for (i = 0; i < n_moves; i++) {
    do_move(pos, move_list[i]);
    move_history[move_history_ptr++] = move_list[i];
    bool ok = false;
    if (ok) {
      printf("%d\n", pos->en_passant_sq[pos->ply]);
      print_board(pos);
      print_bitboard_all(pos);
    }

    U64 cnt = Perft(pos, depth - 1);
    nodes += cnt;
    if (ok) {
      U8 from = move_get_from(move_list[i]);
      U8 to = move_get_to(move_list[i]);
      printf("DEPTH(%d) %d %c%c%c%c: ", depth, move_get_flags(move_list[i]),
             'a' + (from & 7), '1' + (from >> 3), 'a' + (to & 7),
             '1' + (to >> 3));

      printf("%llu\n", cnt);
    }
    if (depth == cool_perft) {
      U8 from = move_get_from(move_list[i]);
      U8 to = move_get_to(move_list[i]);
      printf("DEPTH(%d) %d %c%c%c%c: ", depth, move_get_flags(move_list[i]),
             'a' + (from & 7), '1' + (from >> 3), 'a' + (to & 7),
             '1' + (to >> 3));

      printf("%llu\n", cnt);
    }
    undo_move(pos, move_list[i]);
    move_history_ptr--;
  }
  return nodes;
}

Move random_adversary(Position *pos) {
  Move move_list[256];
  U8 n_moves = gen_legal_moves(pos, move_list);
  if (n_moves == 0 && attacks_on_king) {
    printf("Checkmate! you won!\n");
    exit(0);
  } else if (n_moves == 0) {
    printf("Stalemate!\n");
    exit(0);
  }
  return move_list[rand() % n_moves];
}

Move parse_move(Position *pos, const std::string &s) {
  U8 from = (s[0] - 'a') + (s[1] - '1') * 8;
  U8 to = (s[2] - 'a') + (s[3] - '1') * 8;
  if (board_to_bb(pos->board[from]) == mPawn) {
    if (to - from == 16 || from - to == 16)
      return create_move(from, to, MOVEFLAG_PAWN_DPUSH);
    if (to == pos->en_passant_sq[pos->ply])
      return create_move(from, to, MOVEFLAG_CAPTURE_EP);
    if (pos->side_2_move == mWhite && (to >> 3) == 7) {
      if (pos->board[to]) {
        if (s[4] == 'q')
          return create_move(from, to, MOVEFLAG_PROMOTE_QUEEN_CAPTURE);
        if (s[4] == 'r')
          return create_move(from, to, MOVEFLAG_PROMOTE_ROOK_CAPTURE);
        if (s[4] == 'b')
          return create_move(from, to, MOVEFLAG_PROMOTE_BISHOP_CAPTURE);
        if (s[4] == 'n')
          return create_move(from, to, MOVEFLAG_PROMOTE_KNIGHT_CAPTURE);
      } else {
        if (s[4] == 'q') return create_move(from, to, MOVEFLAG_PROMOTE_QUEEN);
        if (s[4] == 'r') return create_move(from, to, MOVEFLAG_PROMOTE_ROOK);
        if (s[4] == 'b') return create_move(from, to, MOVEFLAG_PROMOTE_BISHOP);
        if (s[4] == 'n') return create_move(from, to, MOVEFLAG_PROMOTE_KNIGHT);
      }
    }
  }
  if (board_to_bb(pos->board[from]) == mKing) {
    if (to == from + 2) return create_move(from, to, MOVEFLAG_CASTLE_KING);
    if (to == from - 2) return create_move(from, to, MOVEFLAG_CASTLE_QUEEN);
  }
  if (pos->board[to]) {
    return create_move(from, to, MOVEFLAG_CAPTURE);
  }
  return create_move(from, to, MOVEFLAG_QUIET);
}

std::string as_string(Move m) {
  std::string s;
  s += 'a' + (move_get_from(m) & 7);
  s += '1' + (move_get_from(m) >> 3);
  s += 'a' + (move_get_to(m) & 7);
  s += '1' + (move_get_to(m) >> 3);
  switch (move_get_flags(m)) {
    case MOVEFLAG_PROMOTE_QUEEN:
    case MOVEFLAG_PROMOTE_QUEEN_CAPTURE:
      s += 'q';
      break;
    case MOVEFLAG_PROMOTE_ROOK:
    case MOVEFLAG_PROMOTE_ROOK_CAPTURE:
      s += 'r';
      break;
    case MOVEFLAG_PROMOTE_BISHOP:
    case MOVEFLAG_PROMOTE_BISHOP_CAPTURE:
      s += 'b';
      break;
    case MOVEFLAG_PROMOTE_KNIGHT:
    case MOVEFLAG_PROMOTE_KNIGHT_CAPTURE:
      s += 'n';
      break;
  }
  return s;
}

int main(void) {
  srand(time(NULL));
  munchkin_init();
  Position *pos = new Position;
  parse_FEN(pos, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
  printf("FEN OK!\n");
  print_board(pos);
  bool is_bot_white = rand() % 2;
  if (is_bot_white) {
    printf("I GO FIRST!\n");
    Move first_move = random_adversary(pos);
    printf("MY MOVE: %s\n", as_string(first_move).c_str());
    do_move(pos, first_move);
    print_board(pos);
  } else {
    printf("YOU GO FIRST!\n");
  }
  char buf[100];
  while (1) {
    printf("YOUR MOVE: ");
    fgets(buf, sizeof(buf), stdin);
    std::string your_move = buf;
    Move m = parse_move(pos, your_move);
    do_move(pos, m);
    print_board(pos);
    Move my_move = random_adversary(pos);
    printf("MY MOVE: %s\n", as_string(my_move).c_str());
    do_move(pos, my_move);
    print_board(pos);
    Move move_list[256];
    U8 n_moves = gen_legal_moves(pos, move_list);
    if (n_moves == 0 && attacks_on_king) {
      printf("Checkmate! I won!\n");
      break;
    } else if (n_moves == 0) {
      printf("Stalemate!");
      break;
    }
  }
  printf("VRAM USED (KB): %d\n", getValueVRAM());
  printf("RAM USED (KB): %d\n", getValueRAM());
  return 0;
}
