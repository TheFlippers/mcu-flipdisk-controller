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

// NEIGHBORS[0] is own id
// XXX first neighbor byte is self id: change this for different display
// controllers
uint8_t NEIGHBORS[5] = { 0x12, 0x02, 0x03, 0x04, 0 };
uint8_t PIXELS[MAX_ROWS] = { 0 };
// uint8_t ID = 0;
uint8_t RECV_NPOS = 0;    // receive neighbor array position
uint8_t SEND_NPOS = 0;    // send neighbor array position
uint8_t RECV_PPOS = 0;    // receive pixel position
uint8_t UPDATE = 0;
enum SPI_STATE RECV_STATE = IDLE;

int main(void) {
  uint8_t next[7] = { 0 };
  uint8_t prev[7] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

  FDDdisplay_init();
  FDDspi_spi_init();
  FDDusart_init();

  FDDdisplay_full(next, prev);
  FDDdisplay_full(prev, next);

  for (int i = 0; i < 7; ++i) {
    prev[i] = next[i];
  }

  next[1] = 0b10000000;
  FDDdisplay_fdither(prev, next);
  while (!UPDATE) {
    int pos[2] = { 1, 0 };
    int momentum[2] = { 1, 1 };
    for (int i = 0; i < 7; ++i) {
      prev[i] = next[i];
    }
    FDDpatterns_bounce(prev, next, momentum, pos);
    FDDdisplay_fdither(prev, next);
  }

  for (;;) {
    if (UPDATE) {
      RECV_PPOS = 0;
      UPDATE = 0;

      for (int i = 0; i < 7; ++i) {
        next[i] = PIXELS[i + 1];
      }
      FDDdisplay_fdither(prev, next);
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
        SPI2->DR = NEIGHBORS[SEND_NPOS++];
        if (SEND_NPOS == 5) {
          RECV_STATE = IDLE;
          SEND_NPOS = 0;
        }
      }
      break;
    }

    case (SEND_NEIGH): {
      if (status & SPI_SR_RXNE) {
        __attribute__((unused)) uint8_t read = SPI2->DR;
      }
      if (status & SPI_SR_TXE) {
        SPI2->DR = NEIGHBORS[SEND_NPOS++];
        if (SEND_NPOS == 5) {
          RECV_STATE = IDLE;
          SEND_NPOS = 0;
        }
      }
      break;
    }

    case (RECV_PIX): {
      if (status & SPI_SR_RXNE) {
        // SPI2->DR = NEIGHBORS[P_POS++];
        PIXELS[RECV_PPOS++] = SPI2->DR;
        if (RECV_PPOS == 7) {
          RECV_PPOS = 0;
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

// TODO need to move to next position if no signals detected
void USART1_IRQHandler() {
  uint32_t status = USART2->SR;

  if (status & USART_SR_RXNE) {
    NEIGHBORS[RECV_NPOS++] = USART2->DR;
  } else if (status & USART_SR_IDLE) {
    USART1->SR &= ~USART_SR_IDLE;
    NEIGHBORS[RECV_NPOS++] = 0;
  }

  if (RECV_NPOS == 5) {
    RECV_NPOS = 1;
  }
  GPIOB->BSRRH = 0b11 << 3;
  GPIOB->BSRRL = RECV_NPOS << 3;
}

void TIM4_IRQHandler() {
  TIM4->SR &= ~(TIM_SR_UIF);
  if (USART1->SR & USART_SR_TC) {
    // USART2->SR &= ~USART_SR_TC;
    USART1->DR = NEIGHBORS[0];
  }
}
