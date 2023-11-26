#include <stdio.h>
#include <string.h>
#include <time.h>

#include "evaluation.h"
#include "munchkin.h"

void perft_benchmark(Position* pos) {
  int n_batches = 7;
  double tot_speed = 0;
  double tot_time = 0;
  U64 tot_nodes = 0;
  for (int i = 1; i <= n_batches; i++) {
    clock_t t = clock();
    U64 res = perft(pos, i, -1);
    t = clock() - t;
    double secs = (double)t / CLOCKS_PER_SEC;
    tot_time += secs;
    double speed = (double)res / secs;
    tot_nodes += res;
    printf(
        "PERFT LEVEL %d: %llu - TOOK ME %.3f milliseconds -> SPEED: %.3f "
        "NODES/SECOND\n",
        i, res, secs * 1000.0, speed);
    tot_speed += speed;
  }
  tot_speed /= (float)n_batches;
  printf("AVERAGE SPEED: %.3f\n", tot_speed);
}

int main(void) {
  srand(time(NULL));
  munchkin_init();
  Position* pos = new Position;
  parse_FEN(pos, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
  printf("FEN OK!\n");
  print_board(pos);
  bool bot_first_move = rand() % 2;
  if (bot_first_move) {
    printf("i go first :3\n");
    Move m = get_best_move(pos, 4);
    printf("MY MOVE:   %s\n", move_as_str(m));
    do_move(pos, m);
  } else {
    printf("u go first :3\n");
  }
  char buf[100];
  while (1) {
    int winner = check_winner(pos);
    if (winner != EVAL_NULL) {
      switch (winner) {
        case EVAL_WINNER_WHITE:
          printf("White won!\n");
          break;
        case EVAL_WINNER_BLACK:
          printf("Black won!\n");
          break;
        case EVAL_STALEMATE:
          printf("Stalemate!\n");
          break;
      }
      break;
    }
    printf("YOUR MOVE: ");
    fgets(buf, sizeof(buf), stdin);
    Move m = parse_move(pos, buf);
    do_move(pos, m);
    winner = check_winner(pos);

    if (winner != EVAL_NULL) {
      switch (winner) {
        case EVAL_WINNER_WHITE:
          printf("White won!\n");
          break;
        case EVAL_WINNER_BLACK:
          printf("Black won!\n");
          break;
        case EVAL_STALEMATE:
          printf("Stalemate!\n");
          break;
      }
      break;
    }
    Move my_move = get_best_move(pos, 4);
    printf("MY MOVE:   %s\n", move_as_str(my_move));
    do_move(pos, my_move);
  }
  return 0;
}
