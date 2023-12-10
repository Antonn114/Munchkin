#include <stdio.h>
#include <time.h>

#include <ios>
#include <iostream>

#include "munchkin.h"
#include "uci.h"

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
  std::ios_base::sync_with_stdio(false);
  std::cin.tie(0);
  munchkin_init();
  Position* pos = new Position;
  parse_FEN(pos, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
  while (true) {
    command_handler(pos);
  }
  return 0;
}
