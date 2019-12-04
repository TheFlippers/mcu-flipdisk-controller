#include "fdd_usart.h"

void FDDusart_init() {
  FDDusart_GPIO_init();
  FDDusart_usart_init();
  FDDusart_timer_init();
}

void FDDusart_GPIO_init() {
  // RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN;
  RCC->AHB1ENR |= /*RCC_AHB1ENR_GPIOAEN |*/ RCC_AHB1ENR_GPIOBEN;

  // PA2 => Tx
  // PA3 => Rx
  // XXX trying pb6 pb7
  // TODO check if PA2/3 work still
  // PB6 => Tx
  // PB7 => Rx
  GPIOB->MODER &= ~(GPIO_MODER_MODER6 | GPIO_MODER_MODER7);
  GPIOB->MODER |= GPIO_MODER_MODER6_1 | GPIO_MODER_MODER7_1;
  GPIOB->AFR[0] &= ~((0xf<<(4*6)) | (0xf<<(4*7)));
  GPIOB->AFR[0] |= (0x07<<(4*6)) | (0x07<<(4*7));

  // GPIOA->MODER &= ~(GPIO_MODER_MODER2 | GPIO_MODER_MODER3);
  // GPIOA->MODER |= GPIO_MODER_MODER2_1 | GPIO_MODER_MODER3_1;
  // GPIOA->AFR[0] &= ~((0xf<<(4*2)) | (0xf<<(4*3)));
  // GPIOA->AFR[0] |= (0x07<<(4*2)) | (0x07<<(4*3));

  // GPIOB 4 and GPIOB 3 used to control mux
  GPIOB->MODER &= ~(GPIO_MODER_MODER3 | GPIO_MODER_MODER4);
  GPIOB->MODER |= GPIO_MODER_MODER3_0 | GPIO_MODER_MODER4_0;
  GPIOB->BSRRH = 0b11<<3;
}

void FDDusart_usart_init() {
  // messages are send LSB
  // baud rate is supposed to be 115200
  //  docs say BRR should have 17.375 (17 | 0b0110)
  //  but the clock source doesn't seem to be quite at
  // 16 MHz so its currently set a little faster to
  // try to better match 115200

  RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
  // RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

  // NVIC->ISER[1] = 1<<6;
  NVIC->ISER[1] = 1<<(USART1_IRQn - 32);
  // NVIC->ISER[1] = 1<<(USART2_IRQn - 31);
  // NVIC->ISER[0] = 1<<USART1_IRQn;

  USART1->CR1 |= USART_CR1_UE;
  USART1->CR1 &= ~(USART_CR1_OVER8 | USART_CR1_M | USART_CR1_PCE
      | USART_CR1_PEIE | USART_CR1_TCIE | USART_CR1_IDLEIE);
  USART1->CR1 |= (USART_CR1_RXNEIE);

  USART1->CR2 &= ~(USART_CR2_LINEN | USART_CR2_STOP | USART_CR2_CLKEN);

  USART1->CR3 &= ~(USART_CR3_HDSEL);

  // XXX this may change with different clock source
  USART1->BRR = (15<<4) | (0b0000);

  USART1->CR1 |= USART_CR1_TE | USART_CR1_RE;
  USART1->DR = 0;
}

void FDDusart_timer_init() {
  RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;
  NVIC->ISER[0] |= 1<<TIM4_IRQn;

  TIM4->CR1 &= ~(TIM_CR1_CKD | TIM_CR1_ARPE | TIM_CR1_CMS | TIM_CR1_DIR
      | TIM_CR1_OPM | TIM_CR1_UDIS);
  // TIM4->CR1 |= TIM_CR1_URS;

  // TIM4->DIER &= ~(TIM_DIER_TDE);
  TIM4->DIER |= TIM_DIER_UIE;

  // TIM4->EGR |= TIM_EGR_UG;

  TIM4->PSC = 16000 - 1;   // 16,000,000 / 16000 = 1000
  TIM4->ARR = 16000 - 1;   // 1000 / 16000 = .0625 s

  TIM4->CR1 |= TIM_CR1_CEN;
}
