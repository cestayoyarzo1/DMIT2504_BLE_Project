/*
  FILE: constants.h
  Created on: 8/12/2017, by Tom Diaz
*/
#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <stm32l4xx.h>

#define BIG_ENDIAN false
#define EVER 1
#define COM_RX_BUFFER_SIZE      1024
#define COM_TX_BUFFER_SIZE      1024
#define TMP_BUFFER_SIZE         1024
#define NUMBER_OF_COM_PORTS     3
#define SPI_PORT_BUFFER_SIZE    40

class COMId
{
public:
  enum Value
  {
    COM_0 = LPUART1_BASE,
    COM_1 = USART1_BASE,
    COM_2 = USART2_BASE,
    COM_3 = USART3_BASE,
  };
};

#endif /* CONSTANTS_H */
