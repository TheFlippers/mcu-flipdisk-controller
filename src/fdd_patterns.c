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

void FDDpatterns_circle(uint8_t* next) {
  static enum SHAPE shape = SQUARE;
  static int draw_cnt = 0;
  static int direction = 0;
  static int pos[2] = { 2, 1};
  static int momentum[2] = { 1, 0 };

  pos[0] += momentum[0];
  pos[1] += momentum[1];

  switch (shape) {
    case SQUARE: {
      if ((pos[0] == 1) && (pos[1] == 1)) {
        momentum[0] = 1;
        momentum[1] = 0;
      } else if ((pos[0] == 5) && (pos[1] == 1)) {
        momentum[0] = 0;
        momentum[1] = 1;
      } else if ((pos[0] == 5) && (pos[1] == 5)) {
        momentum[0] = -1;
        momentum[1] = 0;
      } else if ((pos[0] == 1) && (pos[1] == 5)) {
        momentum[0] = 0;
        momentum[1] = -1;
      }

      /*
      if (pos[0] > 5) { // reached right side from top
        pos[0] = 5;
        pos[1] = 2;
        momentum[0] = 0;
        momentum[1] = 1;
      } else if (pos[0] < 1) { // reached left side of bottom
        pos[0] = 1;
        pos[1] = 4;
        momentum[0] = 0;
        momentum[1] = -1;
      }

      if (pos[1] > 5) { // reached bottom at right side
        pos[0] = 4;
        pos[1] = 5;
        momentum[0] = -1;
        momentum[1] = 0;
      } else if (pos[1] < 1) { // reached top at left side
        pos[0] = 2;
        pos[1] = 1;
        momentum[0] = 1;
        momentum[1] = 0;
      }
      */

      if ((pos[0] == 2) && (pos[1] == 1)) {
        draw_cnt++;
        if (draw_cnt >= 2) {
          draw_cnt = 0;
          shape = CIRCLE;
        }
      }
      break;
    }

    case CIRCLE:{
      if ((pos[0] == 5) && (pos[1] == 1)) {
        pos[0] = 5;
        pos[1] = 2;
        momentum[0] = 0;
        momentum[1] = 1;
      } else if ((pos[0] == 1) && (pos[1] == 5)) {
        pos[0] = 1;
        pos[1] = 4;
        momentum[0] = 0;
        momentum[1] = -1;
      } else if ((pos[0] == 1) && (pos[1] == 1)) {
        pos[0] = 2;
        pos[1] = 1;
        momentum[0] = 1;
        momentum[1] = 0;
      } else if ((pos[0] == 5) && (pos[1] == 5)) {
        pos[0] = 4;
        pos[1] = 5;
        momentum[0] = -1;
        momentum[1] = 0;
      }

      if ((pos[0] == 2) && (pos[1] == 1)) {
        draw_cnt++;
        if (draw_cnt >= 2) {
          draw_cnt = 0;
          if (direction) {
            shape = SQUARE;
            direction = 0;
            momentum[0] = 1;
            momentum[1] = 0;
          } else {
            shape = DIAMOND;
            momentum[0] = 1;
            momentum[1] = -1;
          }
        }
      }
      break;
    }

    case DIAMOND: {
      if ((pos[0] == 3) && (pos[1] == 0)) {
        momentum[1] = 1;
      } else if ((pos[0] == 3) && (pos[1] == 6)) {
        momentum[1] = -1;
      } else if ((pos[0] == 0) && (pos[1] == 3)) {
        momentum[0] = 1;
      } else if ((pos[0] == 6) && (pos[1] == 3)) {
        momentum[0] = -1;
      }

      if ((pos[0] == 2) && (pos[1] == 1)) {
        draw_cnt++;
        if (draw_cnt >= 2) {
          draw_cnt = 0;
          direction = 1;
          shape = CIRCLE;
          momentum[0] = 1;
          momentum[1] = 0;
        }
      }
      break;
    }

    default: {
      shape = SQUARE;
      break;
    }
  }

  next[pos[1]] ^= 2<<pos[0];
}
