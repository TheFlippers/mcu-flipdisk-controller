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
uint8_t NEIGHBORS[5] = { 0, 0x02, 0x03, 0x04, 0 };
uint8_t PIXELS[MAX_ROWS] = { 0 };
// uint8_t ID = 0;
uint8_t NEI_POS = 0;
uint8_t N_POS = 0;
uint8_t P_POS = 0;
uint8_t UPDATE = 0;
enum SPI_STATE RECV_STATE = IDLE;

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
  for (int i = 0; i < 7; ++i) {
    prev[i] = next[i];
  }
  for (;;) {
    if (UPDATE) {
      P_POS = 0;
      N_POS = 0;
      UPDATE = 0;

      for (int i = 0; i < 7; ++i) {
        next[i] = PIXELS[i + 1];
      }
      FDDdisplay_full(prev, next);
      // DMA1->LIFCR = DMA_LIFCR_CTCIF3;
      for (int i = 0; i < 7; ++i) {
        prev[i] = next[i];
      }
    }
  }
}

void SPI2_IRQHandler() {
  uint16_t status = SPI2->SR;
  switch (RECV_STATE) {
    case (IDLE): {
      int ctrl = -1;
      if (status & SPI_SR_RXNE) {
       ctrl = SPI2->DR;
      }
      if (status & SPI_SR_TXE) {
        SPI2->DR = 0;
      }
      switch (ctrl) {
        case (GET_NEIGHBORS): {
          RECV_STATE = GET_NEIGH;
          break;
        }

        case (POST_PIXELS): {
          RECV_STATE = RECV_PIX;
          break;
        }

        case (POST_DISPID): {
          RECV_STATE = RECV_ID;
          break;
        }

        default: {
          RECV_STATE = IDLE;
        }
      }
      break;
    }

    // this should only send zeros. SEND_NEIGH state should be updated by
    // timer interrupt after all neighbor lines have been checked
    case (GET_NEIGH): {
      if (status & SPI_SR_RXNE) {
        __attribute__((unused)) uint8_t read = SPI2->DR;
      }
      if (status & SPI_SR_TXE) {
        // SPI2->DR = 0;
        SPI2->DR = NEIGHBORS[N_POS++];
        if (N_POS == 5) {
          RECV_STATE = IDLE;
          N_POS = 0;
        }
      }
      break;
    }

    case (SEND_NEIGH): {
      if (status & SPI_SR_RXNE) {
        __attribute__((unused)) uint8_t read = SPI2->DR;
      }
      if (status & SPI_SR_TXE) {
        SPI2->DR = NEIGHBORS[N_POS++];
        if (N_POS == 5) {
          RECV_STATE = IDLE;
          N_POS = 0;
        }
      }
      break;
    }

    case (RECV_PIX): {
      if (status & SPI_SR_RXNE) {
        SPI2->DR = NEIGHBORS[P_POS++];
        if (N_POS == 7) {
          N_POS = 0;
          UPDATE = 1;
          RECV_STATE = IDLE;
        }
      }
      if (status & SPI_SR_TXE) {
        SPI2->DR = 0;
      }
      break;
    }

    case (RECV_ID): {
      if (status & SPI_SR_RXNE) {
        // SPI2->DR = NEIGHBORS[0];
        // ID = SPI2->DR;
        NEIGHBORS[0] = SPI2->DR;
        RECV_STATE = IDLE;
      }
      if (status & SPI_SR_TXE) {
        SPI2->DR = 0;
      }
      break;
    }

    default: {
      RECV_STATE = IDLE;
    }
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
  GPIOA->ODR = GPIOA->IDR ^ (1 << 5);
}
