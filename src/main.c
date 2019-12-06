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
// uint8_t NEIGHBORS[MAX_ROWS] = { 0, 2, 3, 4, 5 };
uint8_t PIXELS[MAX_ROWS + 1] = { 0 };
// uint8_t WAIT_CNT = 0;
// uint8_t RECV_NPOS = 0;    // receive neighbor array position
// uint8_t SEND_NPOS = 0;    // send neighbor array position
// uint8_t RECV_PPOS = 0;    // receive pixel position
uint8_t UPDATE = 0;
enum SPI_STATE RECV_STATE = IDLE;

int main(void) {
  uint8_t clear_black[7] = { 0 };
  uint8_t clear_white[7] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

  uint8_t next[7] = { 0 };
  uint8_t prev[7] = { 0 };

  FDDdisplay_init();
  FDDspi_spi_init();
  // FDDusart_init();

  FDDdisplay_fdither(clear_black, clear_white);
  for (int i=0; i<1000000; i++);

  FDDdisplay_fdither(clear_white, clear_black);
  for (int i=0; i<1000000; i++);

  FDDdisplay_fdither(clear_black, clear_white);
  for (int i=0; i<1000000; i++);

  FDDdisplay_fdither(clear_white, clear_black);
  for (int i=0; i<1000000; i++);

  // for (int i = 0; i < 7; ++i) {
  //   prev[i] = next[i];
  // }

  // next[1] = 0b10000000;
  // FDDdisplay_fdither(prev, next);
  while (!UPDATE) {
    FDDpatterns_circle(next);
    FDDdisplay_dither(prev, next);
    for (int i = 0; i < 7; ++i) {
      prev[i] = next[i];
    }
    for (int i=0; i<100000; i++);
  }

  FDDdisplay_fdither(clear_black, clear_white);
  FDDdisplay_fdither(clear_white, clear_black);

  for (int i=0; i < 7; ++i) {
    next[i] = 0;
    prev[i] = 0;
  }

  for (;;) {
    if (UPDATE) {
      UPDATE = 0;

      for (int i = 0; i < 7; ++i) {
        // XXX if pixels gets set back to 7 remve the +1!!!
        next[i] = PIXELS[i + 1];
      }
      // FDDdisplay_full(prev, next);
      FDDdisplay_fdither(prev, next);
      // DMA1->LIFCR = DMA_LIFCR_CTCIF3;
      for (int i = 0; i < 7; ++i) {
        prev[i] = next[i];
      }
    }
  }
}

void SPI2_IRQHandler() {
  static uint8_t pos = 0;
  PIXELS[pos] = SPI2->DR;
  ++pos;
  if (pos == 8) {
    pos = 0;
    UPDATE = 1;
  }
}

/*
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

      // Set next state based on ctrl byte
      switch (ctrl) {
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
          RECV_PPOS = 0;
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
        SPI2->DR = NEIGHBORS[SEND_NPOS++];
        SEND_NPOS %= 8;
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
      RECV_PPOS = 0;
      SEND_NPOS = 0;
    }
  }
}
*/

/*
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
          SEND_NPOS = 0;
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
        // uint8_t self_id = NEIGHBORS[0];
        SPI2->DR = MYID;
        RECV_STATE = SEND_NEIGH;
      }
      break;
    }

    case (SEND_NEIGH): {
      if (status & SPI_SR_RXNE) {
        __attribute__((unused)) uint8_t read = SPI2->DR;
      }
      if (status & SPI_SR_TXE) {
        SPI2->DR = NEIGHBORS[SEND_NPOS++];
        if (SEND_NPOS == 4) {
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
        MYID = SPI2->DR;
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
*/

// TODO need to move to next position if no signals detected
/*
void USART1_IRQHandler() {
  uint32_t status = USART1->SR;

  if (status & USART_SR_RXNE) {
    // NEIGHBORS[RECV_NPOS] = USART1->DR;
    RECV_NPOS = RECV_NPOS + 1;
    if (RECV_NPOS == 5) {
      RECV_NPOS = 1;
    }
    WAIT_CNT = 0;
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

  // Check if neighbors are present after 3 timer counts
  // TODO This needs to be smarter to support `live` reconfiguring
  // maybe store most recent one in value in static variable and if it's the
  // same after 3 receives, then change it back to zero
  ++WAIT_CNT;
  if (WAIT_CNT >= 3) {
    WAIT_CNT = 0;
    // TODO This can be set back to check only 0 for normal operation
    if (NEIGHBORS[RECV_NPOS] == 0
        || NEIGHBORS[RECV_NPOS] == 3) {
      // NEIGHBORS[RECV_NPOS] = 3;
      RECV_NPOS = RECV_NPOS + 1;
      if (RECV_NPOS == 5) {
        RECV_NPOS = 1;
      }
      GPIOB->BSRRH = 0b11 << 3;
      GPIOB->BSRRL = RECV_NPOS << 3;
    }
  }
}
*/
