#include "fdd_display.h"

void FDDdisplay_init() {
  FDDGPIO_init();
  FDDtimer_init();
}

#ifndef __FDD_DIRECT_SEL_H
void FDDGPIO_init() {
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

void FDDtimer_init() {
  RCC->APB1ENR |= RCC_APB1ENR_TIM2EN | RCC_APB1ENR_TIM5EN;
  // CCER -> CCxP = 1 ==> active low
  TIM2->CR1 &= ~(TIM_CR1_CKD | TIM_CR1_ARPE | TIM_CR1_CMS | TIM_CR1_DIR
      | TIM_CR1_UDIS | TIM_CR1_CEN);
  TIM5->CR1 &= ~(TIM_CR1_CKD | TIM_CR1_ARPE | TIM_CR1_CMS | TIM_CR1_DIR
      | TIM_CR1_UDIS | TIM_CR1_CEN);

  TIM2->CR1 |= (TIM_CR1_OPM | TIM_CR1_URS);
  TIM5->CR1 |= (TIM_CR1_OPM | TIM_CR1_URS);

  TIM2->SMCR &= ~TIM_SMCR_SMS;
  TIM5->SMCR &= ~TIM_SMCR_SMS;

  // TODO may not need these
  // TODO may need CC1G/CC2G
  TIM2->EGR |= TIM_EGR_UG;
  TIM5->EGR |= TIM_EGR_UG;

  // TIM2 uses channel 2 for this
  TIM2->CCMR1 &= ~(TIM_CCMR1_OC2CE | TIM_CCMR1_OC2M | TIM_CCMR1_OC2PE
      | TIM_CCMR1_OC2FE | TIM_CCMR1_CC2S);
  TIM2->CCMR1 |= TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2M_1;

  // TIM5 uses channel 1
  TIM5->CCMR1 &= ~(TIM_CCMR1_OC1CE | TIM_CCMR1_OC1M | TIM_CCMR1_OC1PE
      | TIM_CCMR1_OC1FE | TIM_CCMR1_CC1S);
  TIM5->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1;

  TIM2->CCER &= ~TIM_CCER_CC2NP;
  TIM5->CCER &= ~TIM_CCER_CC1NP;

  TIM2->CCER |= TIM_CCER_CC2P | TIM_CCER_CC2E;
  TIM5->CCER |= TIM_CCER_CC1P | TIM_CCER_CC1E;

  TIM2->PSC = FDD_PULSE_PSC;
  TIM5->PSC = FDD_PULSE_PSC;

  TIM2->ARR = FDD_PULSE_ARR;
  TIM5->ARR = FDD_PULSE_ARR;

  TIM2->CCR2 = 2;       // set to active as soon as possible
  TIM5->CCR1 = 2;
}

void FDDdraw(uint8_t* prev, uint8_t* next) {
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

  uint8_t to_black[7];
  uint8_t to_white[7];

  // prev     = 11110000
  // next     = 00001111
  // to_black = 11110000
  // to_white = 00001111
  //
  // prev     = 11001100; ~prev = 00110011;
  // next     = 11110000; ~next = 00001111;
  //
  // to_black = prev & ~next
  // to_white = ~prev & next
  //
  // to_black = 00001100
  // to_white = 00110000

  to_black[0] = prev[0] & ~next[0];
  to_white[0] = ~prev[0] & next[0];

  // GPIOC->BSRRL =
  for(int i=0; i<7; ++i) {
    uint8_t pos = 0x80;

    // TODO check that update event reset CNT and ARR in one pulse mode
    for(int j=0; j<7; ++j) {
      if(to_black[i] & pos) {
        // TODO make sure this is the correct timer to enable
        GPIOC->BSRRH = 0xffff;
        GPIOC->BSRRL = (i<<10) | (j<<5);
        TIM2->PSC = FDD_PULSE_PSC;
        TIM2->ARR = FDD_PULSE_ARR;
        TIM2->CR1 |= TIM_CR1_CEN;
      } else if (to_white[i] & pos) {
        // TODO make sure this is the correct timer to enable
        GPIOC->BSRRH = 0xffff;
        GPIOC->BSRRL = (i<<10) | (j<<5);
        TIM5->PSC = FDD_PULSE_PSC;
        TIM5->ARR = FDD_PULSE_PSC;
        TIM5->CR1 |= TIM_CR1_CEN;
      }
      if(i < 6) {
        to_black[i+1] = prev[i+1] & ~next[i+1];
        to_white[i+1] = ~prev[i+1] & next[i+1];
      }
      pos >>= 1;
      while((TIM2->CR1 & TIM_CR1_CEN) || (TIM5->CR1 & TIM_CR1_CEN));
    }
    GPIOC->BSRRH = 0xffff;
  }
}

#endif
