#include "munchkin.h"
#include <stdio.h>

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

int parseLine(char* line){
    // This assumes that a digit will be found and the line ends in " Kb".
    int i = strlen(line);
    const char* p = line;
    while (*p <'0' || *p > '9') p++;
    line[i-3] = '\0';
    i = atoi(p);
    return i;
}

int getValueVRAM(){ //Note: this value is in KB!
    FILE* file = fopen("/proc/self/status", "r");
    int result = -1;
    char line[128];

    while (fgets(line, 128, file) != NULL){
        if (strncmp(line, "VmSize:", 7) == 0){
            result = parseLine(line);
            break;
        }
    }
    fclose(file);
    return result;
}
int getValueRAM(){ //Note: this value is in KB!
    FILE* file = fopen("/proc/self/status", "r");
    int result = -1;
    char line[128];

    while (fgets(line, 128, file) != NULL){
        if (strncmp(line, "VmRSS:", 6) == 0){
            result = parseLine(line);
            break;
        }
    }
    fclose(file);
    return result;
}
bool ok = false;
bool ok2 = false;
bool ok1 = false;
U64 Perft(Position* pos, int depth){
  Move move_list[256];
  int n_moves, i;
  U64 nodes = 0;
  if (depth == 0) return 1ULL;
  n_moves = pos->gen_legal_moves(move_list);
  for (i = 0; i < n_moves; i++){
    pos->do_move(move_list[i]);
    if (i == 1 && depth == 3) ok1 = ok = true;
    if (ok){
      pos->display_mailbox();
      pos->debug_bitboard(mWhite);
      pos->debug_bitboard(mBlack);
      printf("%d %d %d: ", move_get_from(move_list[i]), move_get_to(move_list[i]), move_get_flags(move_list[i]));
    }
    U64 cnt = Perft(pos, depth - 1);
    nodes += cnt;
    if (ok) printf("%llu\n", cnt);

    pos->undo_move(move_list[i]);
    if (i == 1 && depth == 3) ok1 = ok = false;

  }
  return nodes;
}

int main(void){
  munchkin_init();
  printf("Hello, World!\n");
  Position* pos = new Position;
  pos->parse_FEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 ");
  printf("PERFT: %d %llu\n", 3, Perft(pos, 3));
  getchar();

  printf("VRAM USED (KB): %d\n", getValueVRAM());
  printf("RAM USED (KB): %d\n", getValueRAM());
  return 0;
}
