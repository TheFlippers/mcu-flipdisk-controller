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

int main(void)
{
  uint8_t pixels[8] = { 0 };
  uint8_t neighbors[5] = { DISPLAY_ID, 0 };

  FDDdisplay_init();
  FDDdisplay_spi_init(pixels, neighbors);
	for(;;);
}
