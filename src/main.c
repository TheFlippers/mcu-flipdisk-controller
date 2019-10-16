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

// TODO change these to be very apparent what is sent
uint8_t NEIGHBORS[5] = { 1, 0, 2, 3, 4 };
uint8_t PIXELS[MAX_ROWS+1] = { 0 };
uint8_t N_POS = 0;
uint8_t P_POS = 0;

int main(void) {
  // XXX first neighbor byte is self id: change this for different display
  // controllers
  // TODO fix flow to process first byte from pi


  uint8_t next[7] = { 0 };
  uint8_t prev[7] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
  GPIOA->MODER &= ~GPIO_MODER_MODER5;
  GPIOA->MODER |= GPIO_MODER_MODER5_0;

  FDDdisplay_init();
  FDDspi_init(PIXELS, NEIGHBORS);

  FDDdisplay_draw(prev, next);
  for(int i=0; i<7; ++i) {
    prev[i] = next[i];
  }
  for(;;) {
    if(P_POS == MAX_ROWS + 1) {
      P_POS = 0;
      N_POS = 0;
      GPIOA->BSRRL = 1 << 5;

      for (int i=0; i<7; ++i) {
        next[i] = PIXELS[i+1];
      }
      FDDdisplay_draw(prev, next);
      // DMA1->LIFCR = DMA_LIFCR_CTCIF3;
      for(int i=0; i<7; ++i) {
        prev[i] = next[i];
      }
    }
  }
}

void SPI2_IRQHandler() {
  uint16_t status = SPI2->SR;
  if (status & SPI_SR_TXE) {
    if (N_POS < 5) {
      SPI2->DR = NEIGHBORS[N_POS++];
    }
  }
  if (status & SPI_SR_RXNE) {
      PIXELS[P_POS++] = SPI2->DR;
  }
}
