#include "fdd_patterns.h"

void FDDdesign_ccol_lr(uint8_t* prev, uint8_t* next) {
  for (int j=0; j<7; ++j) { // col
    for (int i=0; i<7; ++i) { // row
      FDDunset_pixel(prev, next, j, i);
    }
    FDDdisplay_draw(prev, next);
  }
}

void FDDdesign_crow_lr(uint8_t* prev, uint8_t* next) {
  for (int i=0; i<7; ++i) { // row
    for (int j=0; j<7; ++j) { // col
      FDDunset_pixel(prev, next, j, i);
    }
    FDDdisplay_draw(prev, next);
  }
}

void FDDdesign_wcol_lr(uint8_t* prev, uint8_t* next) {
  for (int j=0; j<7; ++j) { // col
    for (int i=0; i<7; ++i) { // row
      FDDset_pixel(prev, next, j, i);
    }
    FDDdisplay_draw(prev, next);
  }
}

void FDDdesign_wrow_lr(uint8_t* prev, uint8_t* next) {
  for (int i=0; i<7; ++i) { // row
    for (int j=0; j<7; ++j) { // col
      FDDset_pixel(prev, next, j, i);
    }
    FDDdisplay_draw(prev, next);
  }
}
