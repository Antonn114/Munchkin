#include "piece.h"

inline enumColor toggleColor(enumColor color) {
   return enumColor(color ^ ecBlack);
}
