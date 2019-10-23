#ifndef __FDD_USART_H
#define __FDD_USART_H

#include "stm32f4xx.h"

void FDDusart_init();

void FDDusart_GPIO_init();
void FDDusart_usart_init();
void FDDusart_timer_init();

#endif

