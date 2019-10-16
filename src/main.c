/**
 ******************************************************************************
 * @file    main.c
 * @author  Ac6
 * @version V1.0
 * @date    01-December-2013
 * @brief   Default main function.
 ******************************************************************************
 */


#include "stm32f4xx.h"
#include "main.h"

int main(void) {
  // XXX first neighbor byte is self id: change this for different display
  // controllers
  // TODO fix flow to process first byte from pi
  uint8_t neighbors[5] = { 1, 0 };

  uint8_t pixels[8] = { 0 };
  uint8_t prev[7] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
  uint8_t next[7] = { 0 };

  FDDdisplay_init();
  FDDspi_init(pixels, neighbors);

  for(;;) {
    if(DMA1->LISR & DMA_LISR_TCIF3) {
      memcpy(next, &pixels[1], 7);
      FDDdisplay_draw(prev, next);
      DMA1->LIFCR = DMA_LIFCR_CTCIF3;
      memcpy(prev, next, 7);
    }
  }
}
