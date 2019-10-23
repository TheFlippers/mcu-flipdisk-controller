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
uint8_t NEIGHBORS[5] = { 0, 'x', 'y', 'z', 0 };
uint8_t PIXELS[MAX_ROWS+1] = { 0 };
uint8_t NEI_POS = 1;
uint8_t N_POS = 0;
uint8_t P_POS = 0;
uint8_t UPDATE = 0;

int main(void) {
  // XXX first neighbor byte is self id: change this for different display
  // controllers
  // TODO fix flow to process first byte from pi

  uint8_t next[7] = { 0 };
  uint8_t prev[7] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

  FDDdisplay_init();
  FDDspi_spi_init();
  // FDDusart_init();

  FDDdisplay_draw(prev, next);
  for(int i=0; i<7; ++i) {
    prev[i] = next[i];
  }
  for(;;) {
    if(UPDATE) {
      P_POS = 0;
      N_POS = 0;
      UPDATE = 0;

      for (int i=0; i<7; ++i) {
        next[i] = PIXELS[i+1];
      }
      FDDdisplay_full(prev, next);
      // DMA1->LIFCR = DMA_LIFCR_CTCIF3;
      for(int i=0; i<7; ++i) {
        prev[i] = next[i];
      }
    }
  }
}

void SPI2_IRQHandler() {
  uint16_t status = SPI2->SR;
    if (status & SPI_SR_RXNE) {
      PIXELS[P_POS++] = SPI2->DR;
  }
  if (status & SPI_SR_TXE) {
    if (N_POS < 5) {
      SPI2->DR = NEIGHBORS[N_POS++];
    } else {
      SPI2->DR = 0;
    }
  }
  if (P_POS == 8) {
    UPDATE = 1;
  }
}

void USART2_IRQHandler() {
  uint32_t status = USART2->SR;

  if (status & USART_SR_RXNE) {
    NEIGHBORS[NEI_POS++] = USART2->DR;
    if (NEI_POS == 5) {
      NEI_POS = 1;
    }

    // GPIOA->ODR = GPIOA->IDR ^ (1<<5);
  }
}

void TIM4_IRQHandler() {
  TIM4->SR &= ~(TIM_SR_UIF);
  if (USART2->SR & USART_SR_TC) {
    USART2->DR = NEIGHBORS[0];
  }
  GPIOA->ODR = GPIOA->IDR ^ (1<<5);
}
