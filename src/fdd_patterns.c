#include "fdd_patterns.h"

void FDDpatterns_bounce(uint8_t* prev, uint8_t* next, int* momentum, int* pos) {
  next[pos[0]] = 0;

  for (int i=0; i<2; ++i) {
    pos[i] += momentum[i];
    if (pos[i] >= 7) {
      pos -= 7;
      momentum[i] = -momentum[i];
    } else if (pos[i] < 0) {
      pos[i] = -pos[i];
      momentum[i] = -momentum[i];
    }
  }

  next[pos[0]] = 0x80 >> pos[1];
}

void FDDpatterns_circle(uint8_t* prev, uint8_t* next, int posx, int posy) {
  //
}
