#include "fdd_spi.h"

void FDDspi_init(uint8_t* pixels, uint8_t* neighbors) {
  FDDspi_GPIO_init();
  // FDDspi_dma_init(pixels, neighbors);
  FDDspi_slave_init();
}

void FDDspi_GPIO_init() {
  // gpio for spi communication
  // PB10 => SPI2_SCK
  // PB12 => SPI2_NSS
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;

  GPIOB->MODER &= ~(GPIO_MODER_MODER10 | GPIO_MODER_MODER12);
  GPIOB->MODER |= (GPIO_MODER_MODER10_1 | GPIO_MODER_MODER12_1);

  // odd that GPIO_AFR(L/H)_AFSELx isn't defined
  GPIOB->AFR[1] &= ~(0xF<<(4*2) | 0xF<<(4*4));
  GPIOB->AFR[1] |= (5<<(4*2)) | (5<<(4*4));

  GPIOB->OSPEEDR |= 0b11<<(2*10) | 0b11<<(2*12);

  // PC2 => SPI2_MISO
  // PC3 => SPI2_MOSI
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;

  GPIOC->MODER &= ~(GPIO_MODER_MODER2 | GPIO_MODER_MODER3);
  GPIOC->MODER |= GPIO_MODER_MODER2_1 | GPIO_MODER_MODER3_1;

  GPIOC->AFR[0] &= ~(0xF<<(4*2) | 0xF<<(4*3));
  GPIOC->AFR[0] |= (5<<(4*2)) | (5<<(4*3));

  GPIOC->OSPEEDR |= 0b11<<(2*2) | 0b11<<(2*3);
}

void FDDspi_slave_init() {
  RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
  NVIC->ISER[1] = 1<<(SPI2_IRQn - 32);

  while(SPI2->SR & SPI_SR_BSY);

  SPI2->CR1 &= ~SPI_CR1_SPE;

  // 2-line unidirectional, disable crc calc, data frame formate: 8 bits
  // full-duplex, hardware slave management, MSB, fastest baud rate (fpclk / 2)
  // slave config, first clock = first data edge
  SPI2->CR1 &= ~(
      SPI_CR1_BIDIMODE | SPI_CR1_CRCEN | SPI_CR1_CRCNEXT | SPI_CR1_DFF
      | SPI_CR1_RXONLY | SPI_CR1_SSM | SPI_CR1_LSBFIRST | SPI_CR1_BR
      | SPI_CR1_MSTR | SPI_CR1_CPOL | SPI_CR1_CPHA);

  SPI2->CR2 |= SPI_CR2_TXEIE | SPI_CR2_RXNEIE;

  SPI2->CR1 |= SPI_CR1_SPE;
}

void FDDspi_slave_wDMA_init() {
  RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
  // NVIC->ISER[1] = 1<<(SPI2_IRQn - 32);

  while(SPI2->SR & SPI_SR_BSY);

  SPI2->CR1 &= ~SPI_CR1_SPE;

  // 2-line unidirectional, disable crc calc, data frame formate: 8 bits
  // full-duplex, hardware slave management, MSB, fastest baud rate (fpclk / 2)
  // slave config, first clock = first data edge
  SPI2->CR1 &= ~(
      SPI_CR1_BIDIMODE | SPI_CR1_CRCEN | SPI_CR1_CRCNEXT | SPI_CR1_DFF
      | SPI_CR1_RXONLY | SPI_CR1_SSM | SPI_CR1_LSBFIRST | SPI_CR1_BR
      | SPI_CR1_MSTR | SPI_CR1_CPOL | SPI_CR1_CPHA);

  SPI2->CR2 |= SPI_CR2_RXDMAEN | SPI_CR2_TXDMAEN;

  // SPI2->CR2 |= SPI_CR2_TXEIE | SPI_CR2_RXNEIE;

  SPI2->CR1 |= SPI_CR1_SPE;
}

void FDDspi_dma_init(uint8_t* pixels, uint8_t* neighbors) {
  RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;

  // TODO Should these be enabled after receiving the first (control) byte
  // from the pi?

  DMA1_Stream3->CR &= ~(
      DMA_SxCR_CHSEL | DMA_SxCR_MSIZE | DMA_SxCR_PSIZE |
      DMA_SxCR_PINC | DMA_SxCR_DIR);

  DMA1_Stream3->CR |= (
      DMA_SxCR_MINC /*| DMA_SxCR_CIRC*/ | DMA_SxCR_PFCTRL |
      DMA_SxCR_TCIE);

  // TODO this should change when first byte processing is finished
  DMA1_Stream3->NDTR = 8;
  DMA1_Stream3->M0AR = (uint32_t) pixels;
  DMA1_Stream3->PAR = (uint32_t) &(SPI2->DR);

  NVIC->ISER[0] |= 1<<DMA1_Stream3_IRQn;


  DMA1_Stream4->CR &= ~(
      DMA_SxCR_CHSEL | DMA_SxCR_MSIZE | DMA_SxCR_PSIZE |
      DMA_SxCR_PINC | DMA_SxCR_DIR);

  DMA1_Stream4->CR |= (
      DMA_SxCR_MINC /*| DMA_SxCR_CIRC*/ | DMA_SxCR_DIR_0 | DMA_SxCR_PFCTRL);

  DMA1_Stream4->NDTR = 5;
  DMA1_Stream4->M0AR = (uint32_t) neighbors;
  DMA1_Stream4->PAR = (uint32_t) &(SPI2->DR);

  // NVIC->ISER[0] |= 1<<DMA1_Stream4_IRQn;
  DMA1_Stream3->CR |= DMA_SxCR_EN;
  DMA1_Stream4->CR |= DMA_SxCR_EN;
}
