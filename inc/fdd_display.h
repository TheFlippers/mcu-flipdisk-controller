#ifndef __FLIP_DISPLAY_DRIVER_H
#define __FLIP_DISPLAY_DRIVER_H

#include "stm32f4xx.h"

// PC5 => RADDR0
// PC6 => RADDR1
// PC7 => RADDR2

// PC10 => CADDR0
// PC11 => CADDR1
// PC12 => CADDR2

// TODO ensure these are the correct output lines
#ifndef __FDD_DIRECT_SEL_H
#define FDD_ROW_ADDR0 (1<<5)
#define FDD_ROW_ADDR1 (1<<6)
#define FDD_ROW_ADDR2 (1<<7)

#define FDD_COL_ADDR0 (1<<10)
#define FDD_COL_ADDR1 (1<<11)
#define FDD_COL_ADDR2 (1<<12)
#endif

#define FDD_PULSE_PSC (160 - 1)
#define FDD_PULSE_ARR (160 - 1)
// #ifdef __FDD_DIRECT_SEL_H
// // TODO select outputs in case muxes aren't working
// #endif

void FDDdisplay_init();
void FDDdisplay_GPIO_init();
void FDDdisplay_timer_init();
void FDDdisplay_draw(uint8_t*, uint8_t*);

#endif
