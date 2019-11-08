#include "fdd_spi.h"
#include "fdd_display.h"
#include "fdd_usart.h"
#include "fdd_patterns.h"

#define MAX_ROWS 7

// Get
#define GET_NEIGHBORS 0b10000000
#define POST_NEIGHBORS 0b00000000 // should never be received expected to be sent from micro
#define POST_PIXELS 0b00000001
#define POST_DISPID 0b00000010

enum SPI_STATE {
  IDLE = 0,
  RECV_ID,
  RECV_PIX,
  GET_NEIGH,
  SEND_NEIGH
};
