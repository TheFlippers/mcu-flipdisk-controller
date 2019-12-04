#include "fdd_display.h"

const int RC[49][2] = {
// const int RC[64][2] = {
  {3, 2},
  {5, 4},
  {1, 6},
  {5, 2},
  {3, 3},
  {4, 1},
  {1, 5},
  {6, 1},
  {5, 5},
  {0, 6},
  {5, 0},
  {6, 4},
  {6, 2},
  {0, 4},
  {3, 0},
  {0, 1},
  {2, 5},
  {3, 6},
  {2, 1},
  {0, 2},
  {5, 6},
  {0, 5},
  {1, 2},
  {6, 6},
  {4, 4},
  {5, 1},
  {2, 3},
  {0, 0},
  {4, 2},
  {6, 5},
  {6, 0},
  {1, 3},
  {1, 0},
  {3, 5},
  {2, 4},
  {4, 0},
  {1, 4},
  {1, 1},
  {4, 6},
  {2, 6},
  {3, 4},
  {4, 5},
  {0, 3},
  {2, 0},
  {6, 3},
  {2, 2},
  {3, 1},
  {4, 3},
  {5, 3}
};
/* ,

  {0, 7},
  {1, 7},
  {2, 7},
  {3, 7},
  {4, 7},
  {5, 7},
  {6, 7},
  {7, 7},
  {7, 6},
  {7, 5},
  {7, 4},
  {7, 3},
  {7, 2},
  {7, 1},
  {7, 0}
};
*/

void FDDdisplay_init() {
  FDDdisplay_GPIO_init();
  FDDdisplay_timer_init();
}

#ifndef __FDD_DIRECT_SEL_H
void FDDdisplay_GPIO_init() {
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOCEN;

  // PA0 and PA1 are enables controlled by timers

  GPIOA->MODER &= ~(GPIO_MODER_MODER0 | GPIO_MODER_MODER1);
  GPIOA->MODER |= GPIO_MODER_MODER0_1 | GPIO_MODER_MODER1_1;

  GPIOA->AFR[0] &= ~(0xf << (4*0) | 0xf << (4*1));
  GPIOA->AFR[0] |= (0b0010 << (4*0)) | (0b0001 << (4*1));

  // pull-up resistors to keep high whlie timers are disabled
  // GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPDR0 | GPIO_PUPDR_PUPDR1);
  // GPIOA->PUPDR |= GPIO_PUPDR_PUPDR0_0 | GPIO_PUPDR_PUPDR1_0;

  GPIOC->MODER &= ~(GPIO_MODER_MODER5 | GPIO_MODER_MODER6 | GPIO_MODER_MODER7
      | GPIO_MODER_MODER10 | GPIO_MODER_MODER11 | GPIO_MODER_MODER12);

  GPIOC->MODER |= (GPIO_MODER_MODER5_0 | GPIO_MODER_MODER6_0
      | GPIO_MODER_MODER7_0 | GPIO_MODER_MODER10_0 | GPIO_MODER_MODER11_0
      | GPIO_MODER_MODER12_0);
}

void FDDdisplay_timer_init() {
  RCC->APB1ENR |= RCC_APB1ENR_TIM2EN | RCC_APB1ENR_TIM3EN | RCC_APB1ENR_TIM5EN;

  // CCER -> CCxP = 1 ==> active low
  TIM2->CR1 &= ~(TIM_CR1_CKD | TIM_CR1_ARPE | TIM_CR1_CMS | TIM_CR1_DIR
      | TIM_CR1_UDIS | TIM_CR1_CEN);
  TIM3->CR1 &= ~(TIM_CR1_CKD | TIM_CR1_ARPE | TIM_CR1_CMS | TIM_CR1_DIR
      | TIM_CR1_UDIS | TIM_CR1_CEN);
  TIM5->CR1 &= ~(TIM_CR1_CKD | TIM_CR1_ARPE | TIM_CR1_CMS | TIM_CR1_DIR
      | TIM_CR1_UDIS | TIM_CR1_CEN);

  TIM2->CR1 |= (TIM_CR1_OPM | TIM_CR1_URS);
  TIM3->CR1 |= (TIM_CR1_OPM | TIM_CR1_URS);
  TIM5->CR1 |= (TIM_CR1_OPM | TIM_CR1_URS);

  TIM2->SMCR &= ~TIM_SMCR_SMS;
  TIM3->SMCR &= ~TIM_SMCR_SMS;
  TIM5->SMCR &= ~TIM_SMCR_SMS;

  // TODO may not need these
  // TODO may need CC1G/CC2G
  TIM2->EGR |= TIM_EGR_UG;
  TIM3->EGR |= TIM_EGR_UG;
  TIM5->EGR |= TIM_EGR_UG;

  // TIM2 uses channel 2 for this
  TIM2->CCMR1 &= ~(TIM_CCMR1_OC2CE | TIM_CCMR1_OC2M | TIM_CCMR1_OC2PE
      | TIM_CCMR1_OC2FE | TIM_CCMR1_CC2S);
  TIM2->CCMR1 |= TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2M_1;

  // TIM5 uses channel 1
  TIM5->CCMR1 &= ~(TIM_CCMR1_OC1CE | TIM_CCMR1_OC1M | TIM_CCMR1_OC1PE
      | TIM_CCMR1_OC1FE | TIM_CCMR1_CC1S);
  TIM5->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1;

  // TIM3 doesn't want to output anything during this time

  TIM2->CCER &= ~TIM_CCER_CC2NP;
  TIM5->CCER &= ~TIM_CCER_CC1NP;

  TIM2->CCER |= TIM_CCER_CC2P | TIM_CCER_CC2E;
  TIM5->CCER |= TIM_CCER_CC1P | TIM_CCER_CC1E;

  TIM2->PSC = FDD_PULSE_PSC;
  TIM3->PSC = FDD_PULSE_PSC;
  TIM5->PSC = FDD_PULSE_PSC;

  TIM2->ARR = FDD_PULSE_ARR;
  TIM3->ARR = FDD_PULSE_ARR;
  TIM5->ARR = FDD_PULSE_ARR;

  TIM2->CCR2 = 2;       // set to active as soon as possible
  TIM5->CCR1 = 2;
}

void FDDdisplay_draw(uint8_t* prev, uint8_t* next) {
  // TODO finish draw function
  // process:
  // 1. ensure driver output enables all 0
  // 2. update shift regs
  //    a. set data lines to next output
  //    b. pulse clock signal
  //    c. perform 7x total
  // 3. pulse shift enable
  // 4. enable driver outputs
  //    a. row data should always be enabled
  //    b. only one of colomn data should be enabled at a time

  uint8_t to_black;
  uint8_t to_white;

  for(int i=0; i<7; ++i) {
    // uint8_t pos = 0b01000000;

    for(int j=0; j<7; ++j) {
      to_black = prev[i] & ~next[i] & 2<<j;
      to_white = ~prev[i] & next[i] & 2 << j;

      while((TIM2->CR1 & TIM_CR1_CEN) || (TIM5->CR1 & TIM_CR1_CEN));

      if(to_black) {
        // BSRRH corresponds to reset output
        GPIOC->BSRRH = 0xffff;
        // BSRRL corresponds to set output
        GPIOC->BSRRL = (i<<10) | (j<<5);
        TIM2->CR1 |= TIM_CR1_CEN;
      } else if (to_white) {
        GPIOC->BSRRH = 0xffff;
        GPIOC->BSRRL = (i<<10) | (j<<5);
        TIM5->CR1 |= TIM_CR1_CEN;
      }
    }
    GPIOC->BSRRH = 0xffff;
  }
}

void FDDdisplay_full(uint8_t* prev, uint8_t* next) {
  // TODO finish draw function
  // process:
  // 1. ensure driver output enables all 0
  // 2. update shift regs
  //    a. set data lines to next output
  //    b. pulse clock signal
  //    c. perform 7x total
  // 3. pulse shift enable
  // 4. enable driver outputs
  //    a. row data should always be enabled
  //    b. only one of colomn data should be enabled at a time

  uint8_t to_black;
  uint8_t to_white;

  for(int i=0; i<7; ++i) {
  // for(int j=0; j<7; ++j) {
    for(int j=0; j<7; ++j) {
    // for(int i=0; i<7; ++i) {
      to_black = prev[i] & ~next[i] & 2<<j;
      to_white = ~prev[i] & next[i] & 2<<j;

      while((TIM2->CR1 & TIM_CR1_CEN)
          || (TIM5->CR1 & TIM_CR1_CEN)
          || (TIM3->CR1 & TIM_CR1_CEN)
      );

      if(to_black) {
        // BSRRH corresponds to reset output
        GPIOC->BSRRH = 0xffff;
        // BSRRL corresponds to set output
        GPIOC->BSRRL = (i<<10) | (j<<5);
        TIM2->CR1 |= TIM_CR1_CEN;
      } else if (to_white) {
        GPIOC->BSRRH = 0xffff;
        GPIOC->BSRRL = (i<<10) | (j<<5);
        TIM5->CR1 |= TIM_CR1_CEN;
      } else {
        TIM3->CR1 |= TIM_CR1_CEN;
      }
    }
    GPIOC->BSRRH = 0xffff;
    // for (int k=0; k<10000; ++k);
  }
}

void FDDdisplay_dither(uint8_t* prev, uint8_t* next) {
  uint8_t to_black, to_white;
  for (int i = 0; i < sizeof(RC) / sizeof(RC[0]); ++i) {
    // This should take care of setting up to_black and to_white if that pixel
    // is supposed to be changed to either of those colors
    // position is not needed because it is replaced with the last value of in
    // the following bitwise and. This is done because the position doesn't
    // just increment as it was done in the previous draw functions.
                                                // 0b00000010
    to_black = prev[RC[i][0]] & ~next[RC[i][0]] & (2<<RC[i][1]);
    to_white = ~prev[RC[i][0]] & next[RC[i][0]] & (2<<RC[i][1]);

    while((TIM2->CR1 & TIM_CR1_CEN)
        || (TIM5->CR1 & TIM_CR1_CEN)
    );

    if(to_black) {
      // BSRRH corresponds to reset output
      GPIOC->BSRRH = (0b111<<10) | (0b111<<5);
      // BSRRL corresponds to set output
      GPIOC->BSRRL = (RC[i][0]<<10) | (RC[i][1]<<5);
      TIM2->CR1 |= TIM_CR1_CEN;
    } else if (to_white) {
      GPIOC->BSRRH = (0b111<<10) | (0b111<<5);
      GPIOC->BSRRL = (RC[i][0]<<10) | (RC[i][1]<<5);
      TIM5->CR1 |= TIM_CR1_CEN;
    }
  }
}

void FDDdisplay_fdither(uint8_t* prev, uint8_t* next) {
  uint8_t to_black, to_white;
  for (int i = 0; i < 49; ++i) {                                                // 0b00000010
    to_black = prev[RC[i][0]] & ~next[RC[i][0]] & (2<<RC[i][1]);
    to_white = ~prev[RC[i][0]] & next[RC[i][0]] & (2<<RC[i][1]);

    while((TIM2->CR1 & TIM_CR1_CEN)
        || (TIM5->CR1 & TIM_CR1_CEN)
        || (TIM3->CR1 & TIM_CR1_CEN)
    );

    if(to_black) {
      GPIOC->BSRRH = (0b111<<10) | (0b111<<5);
      GPIOC->BSRRL = (RC[i][0]<<10) | (RC[i][1]<<5);
      TIM2->CR1 |= TIM_CR1_CEN;
    } else if (to_white) {
      GPIOC->BSRRH = (0b111<<10) | (0b111<<5);
      GPIOC->BSRRL = (RC[i][0]<<10) | (RC[i][1]<<5);
      TIM5->CR1 |= TIM_CR1_CEN;
    } else {
      GPIOC->BSRRH = (0b111<<10) | (0b111<<5);
      TIM3->CR1 |= TIM_CR1_CEN;
    }
  }
  // for (int i=0; i<50000; i++);
}

void FDDdisplay_drawallthedotswhite() {
  for (int i=0; i<7; ++i) {
    for (int j=0; j<7; ++j) {
      while((TIM2->CR1 & TIM_CR1_CEN)
          || (TIM5->CR1 & TIM_CR1_CEN)
          || (TIM3->CR1 & TIM_CR1_CEN)
      );
      for (int k=0; k<100000; k++);
      GPIOC->BSRRH = (0b111<<10) | (0b111<<5);
      GPIOC->BSRRL = (i<<10) | (j<<5);
      TIM5->CR1 |= TIM_CR1_CEN;

    }
  }
}

void FDDdisplay_drawallthedotsblack() {
  for (int i=0; i<7; ++i) {
    for (int j=0; j<7; ++j) {
      while((TIM2->CR1 & TIM_CR1_CEN)
          || (TIM5->CR1 & TIM_CR1_CEN)
          || (TIM3->CR1 & TIM_CR1_CEN)
      );
      for (int k=0; k<100000; k++);
      GPIOC->BSRRH = (0b111<<10) | (0b111<<5);
      GPIOC->BSRRL = (i<<10) | (j<<5);
      TIM2->CR1 |= TIM_CR1_CEN;

    }
  }
}

// utility functions
void FDDset_pixel(uint8_t* prev, uint8_t* next, int col, int row) {
  next[row] = prev[row] | (0b01000000>>col);
}

void FDDunset_pixel(uint8_t* prev, uint8_t* next, int col, int row) {
  next[row] = prev[row] & ~(0b01000000>>col);
}

#endif
