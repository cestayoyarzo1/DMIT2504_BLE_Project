/*
  FILE: hal.cpp
  Created on: 8/12/2017, by Tom Diaz
*/

#include <hal.h>
#include <stdio.h>
#include <stdarg.h>

#define SYSCLK 80000000

static SysL476xx* sysPtr = 0;

void HAL::Init(void* sys)
{
  if(!sysPtr) // quasi-singleton pattern
  {
    __disable_irq();
    
    
    Clock();
    Uart();
    //SPI();
    //I2c();
    
    sysPtr = (SysL476xx*)sys;
    
    
    __enable_irq(); 
    sysPtr->Init();
  }
}
//------------------------------------------------------------------------------
void HAL::Clock()
{
  //Enable peripherals
  RCC->APB1ENR1 |= RCC_APB1ENR1_PWREN;  //Enable power interface
  
  //Enable instruction prefetch, data and instruction cache
  FLASH->ACR |= FLASH_ACR_PRFTEN | FLASH_ACR_ICEN | FLASH_ACR_DCEN;
  
  //Slowdown FLASH
  FLASH->ACR &= ~FLASH_ACR_LATENCY;
  FLASH->ACR |= FLASH_ACR_LATENCY_1WS;
  
  //System will be running from MSI trimmed from LSE
  
  //Configure LSE
  PWR->CR1 |= PWR_CR1_DBP;                      //Enable backup domain write
  
  RCC->BDCR &= ~RCC_BDCR_BDRST;                 //Do NOT reset backup domain
  RCC->BDCR &= ~RCC_BDCR_LSEBYP;                //No LSE Bypass (use crystal)
  RCC->BDCR |= RCC_BDCR_LSEON;                  //LSE ON
  while(!(RCC->BDCR & RCC_BDCR_LSERDY));        //Wait until LSE is Ready
  
  RCC->BDCR |= RCC_BDCR_RTCSEL_0;             //LSE as RTC clock
  RCC->BDCR |= RCC_BDCR_RTCEN;                //Enable RTC
  
  PWR->CR1 &= ~PWR_CR1_DBP;                     //Disable backup domain write
  
  //Configure MSI
  RCC->CR &= ~RCC_CR_MSION;             //Turn MSI Off
  RCC->CR &= ~RCC_CR_MSIPLLEN;          //Disable MSI PLL
  RCC->CR |= RCC_CR_MSIRGSEL;           //Take Frequency Range from RCC->CR  
  RCC->CR &= ~RCC_CR_MSIRANGE;          //Clear Range
  RCC->CR |= RCC_CR_MSIRANGE_6;        //4Mhz Range
  RCC->CR |= RCC_CR_MSIPLLEN;           //Enable MSI PLL (Auto trim by LSE)
  RCC->CR |= RCC_CR_MSION;              //Turn On MSI
  while(!(RCC->CR & RCC_CR_MSIRDY));    //Wait until MSI is stable
  
  RCC->PLLCFGR |= RCC_PLLCFGR_PLLSRC_MSI; //MSI as PLL clock Source
  RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLM;    //Clear PLLM, M = 1
  RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLN;     //Clear PLLN
  RCC->PLLCFGR |= RCC_PLLCFGR_PLLN_3 | RCC_PLLCFGR_PLLN_5; //N = 40
                                        // PLLR is 2 after reset
  
  RCC->CFGR &= ~RCC_CFGR_MCOSEL;        //Clear MCOSEL
  //RCC->CFGR |= RCC_CFGR_MCOSEL_1;     //MSI as MCO
  RCC->CFGR |= RCC_CFGR_MCOSEL_0;       //SysCLK as MCO = 80 Mhz
  RCC->CFGR |= RCC_CFGR_MCO_PRE_16;       // MCO / 16 = 5 Mhz
    
  /*
  // USB clocks begin
  RCC->PLLSAI1CFGR = 0x00101800;        //TODO Expand this
  RCC->CR |= RCC_CR_PLLSAI1ON;          //Turn On PLLSAI1
  while(!(RCC->CR & RCC_CR_PLLSAI1RDY));//Wait until PLLSAI1 is stable
  RCC->CCIPR |= RCC_CCIPR_CLK48SEL_0;
  // USB clocks end
  */
  
  RCC->CR |= RCC_CR_PLLON;              //Turn On PLL
  while(!RCC->CR & RCC_CR_PLLRDY);      //Wait until PLL is locked
  RCC->PLLCFGR |= RCC_PLLCFGR_PLLREN;   //Enable PLL Output
  
  RCC->CFGR &= ~RCC_CFGR_SW;            //Clear SySClk selection (fallback to MSI)
  RCC->CFGR |= RCC_CFGR_SW_0 | RCC_CFGR_SW_1; //PLL as SYSCLK
  
  SysTick_Config(SYSCLK / 1000); // 80Mhz / 1000 = 80Khz, 1ms Ticks
  NVIC_EnableIRQ(SysTick_IRQn);
  
  RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;  //Enable PORT A
  RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;  //Enable PORT B
  RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN;  //Enable PORT C
  
  RCC->APB2ENR |= RCC_APB2ENR_USART1EN; // Enable UART 1
  RCC->APB1ENR1 |= RCC_APB1ENR1_USART2EN; // Enable UART 2
  RCC->APB1ENR1 |= RCC_APB1ENR1_USART3EN; // Enable UART 3
  
  /*
  GPIOC->MODER &= ~GPIO_MODER_MODER0;
  GPIOC->MODER |= GPIO_MODER_MODER0_0;
  //GPIOC->BSRR |= GPIO_BSRR_BS_0;
  GPIOC->BRR |= GPIO_BRR_BR_0;
  */
  
  GPIOC->MODER &= ~GPIO_MODER_MODER0;
  GPIOC->MODER |= GPIO_MODER_MODER0;
  //GPIOC->BSRR |= GPIO_BSRR_BS_0;
  //GPIOC->BRR |= GPIO_BRR_BR_0;
}
//-------------------------------------------------------------------------CRC32
uint32_t HAL::CRC32(void* data, uint32_t len, uint32_t firstcrc)
{
  uint32_t ans = 0;
  uint8_t* dataP = (uint8_t*)data;
  
  RCC->AHB1ENR |= RCC_AHB1ENR_CRCEN;    //Enable CRC unit
  CRC->CR |= 0x01;      // Reset peripheral
  
  CRC->INIT = firstcrc;     // Initial CRC value
  CRC->CR &= ~(7<<5);      // No bit order reversal 
  CRC->CR &= ~(3<<3);      // 32 bit CRC
  // Default polynomial (Ethernet)
  
  for(uint32_t i = 0; i<len; i++)
    *(uint8_t __IO *)(&CRC->DR) = dataP[i];
  
  ans = CRC->DR;
  
  RCC->AHB1ENR &= ~RCC_AHB1ENR_CRCEN;    //Disable CRC unit
  
  return ans;
}
//---------------------------------------------------------------------------I2c
void HAL::I2c()
{   
  GPIOB->MODER &= ~GPIO_MODER_MODER7;           // Clear MODER settings
  GPIOB->MODER |= GPIO_MODER_MODER7_1;          // Alternate function SDA
  GPIOB->AFR[0] |= 1<<28;                        // Alternate function 1
  GPIOB->OTYPER |= GPIO_OTYPER_OT_7;            // Open-drain output
  GPIOB->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR7;      // High speed
  //GPIOB->PUPDR &= ~GPIO_PUPDR_PUPD7;           // Clear resistor
  //GPIOB->PUPDR |= GPIO_PUPDR_PUPDR7_0;          // Enable Pull-up

  GPIOB->MODER &= ~GPIO_MODER_MODER6;           // Clear MODER settings
  GPIOB->MODER |= GPIO_MODER_MODER6_1;          // Alternate function SCL
  GPIOB->AFR[0] |= 1<<24;                        // Alternate function 1
  GPIOB->OTYPER |= GPIO_OTYPER_OT_6;            // Open-drain output
  GPIOB->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR6;      // High speed
  //GPIOB->PUPDR &= ~GPIO_PUPDR_PUPDR6;           // Clear resistor
  //GPIOB->PUPDR |= GPIO_PUPDR_PUPDR6_0;        // Enable Pull-up
  
  RCC->APB1ENR1 |= RCC_APB1ENR1_I2C1EN;   //Enable I2C1
  while(!(RCC->APB1ENR1 & RCC_APB1ENR1_I2C1EN));
  
  I2C1->TIMINGR = 0x00702991;               // 400Khz @ 80Mhz as per STM32CubeMX
  I2C1->OAR1 |= I2C_OAR1_OA1EN;
  I2C1->CR2 |= I2C_CR2_AUTOEND;
  
  I2C1->CR1 |= I2C_CR1_PE;                      // Enable peripheral
}
//------------------------------------------------------------------------SysClk
uint32_t HAL::SysClk()
{
  return SYSCLK;
}
//------------------------------------------------------------------------------
void HAL::Debug(const char* format, ...)
{
  va_list arg; 
  va_start(arg, format); 
  
  sysPtr->debugUart.Printf(format,arg);
  vprintf(format,arg);
  
  va_end(arg);
}
//----------------------------------Switch Uart2 from VCOM to Pins, Nucleo Board
void HAL::SwitchU2Tx(ConnectionSource::Value source)
{
  GPIOA->MODER &= ~GPIO_MODER_MODER3;  //Clear MODER
  GPIOA->MODER &= ~GPIO_MODER_MODER15;  //Clear MODER
  switch(source)
  {
  case ConnectionSource::RS232:
    //GPIOA->MODER &= ~GPIO_MODER_MODER3;  //Clear MODER
    GPIOA->MODER |= GPIO_MODER_MODER3_1; //Set alternate function
    GPIOA->AFR[0] &= ~(7<<12);					// Clear AF
    GPIOA->AFR[0] |= 7<<12;             //Set alternate function 4 (UART)
    break;
  case ConnectionSource::STLink:
    //GPIOA->MODER &= ~GPIO_MODER_MODER15;  //Clear MODER
    GPIOA->MODER |= GPIO_MODER_MODER15_1; //Set alternate function
    GPIOA->AFR[1] &= ~(3<<28);					// Clear AF
    GPIOA->AFR[1] |= 3<<28;             //Set alternate function 4 (UART)
    break;
  case ConnectionSource::None:
    break;
  }
}
//--------------------------------------------------------------------------Uart
void HAL::Uart()
{
  //UART3
  // TX
  GPIOC->MODER &= ~GPIO_MODER_MODER4;  //Clear MODER
  GPIOC->MODER |= GPIO_MODER_MODER4_1; //Set alternate function
  GPIOC->AFR[0] |= 7<<16;              //Set alternate function 7 (UART)
  
  // RX
  GPIOC->MODER &= ~GPIO_MODER_MODER5;  //Clear MODER
  GPIOC->MODER |= GPIO_MODER_MODER5_1; //Set alternate function
  GPIOC->AFR[0] |= 7<<20;               //Set alternate function 7 (UART)
  
  //Important!!! no floating inputs TX  pull high and RX pull low
  GPIOC->PUPDR |= GPIO_PUPDR_PUPDR4_0 | GPIO_PUPDR_PUPDR5_1;
  
  NVIC_EnableIRQ(USART3_IRQn); // CM4 Intrinsic
  USART3->CR1 |= USART_CR1_RXNEIE;
  
  //UART2
  // TX
  GPIOA->MODER &= ~GPIO_MODER_MODER2;  //Clear MODER
  GPIOA->MODER |= GPIO_MODER_MODER2_1; //Set alternate function
  GPIOA->AFR[0] |= 7<<8;              //Set alternate function 7 (UART)
  
  // RX
  GPIOA->MODER &= ~GPIO_MODER_MODER3;  //Clear MODER
  GPIOA->MODER |= GPIO_MODER_MODER3_1; //Set alternate function
  GPIOA->AFR[0] |= 7<<12;               //Set alternate function 7 (UART)
  
  //Important!!! no floating inputs TX  pull high and RX pull low
  GPIOA->PUPDR |= GPIO_PUPDR_PUPDR2_0 | GPIO_PUPDR_PUPDR15_1;
  
  NVIC_EnableIRQ(USART2_IRQn); // CM4 Intrinsic
  USART2->CR1 |= USART_CR1_RXNEIE;
  
  //UART1
  // TX
  GPIOA->MODER &= ~GPIO_MODER_MODER9;  //Clear MODER
  GPIOA->MODER |= GPIO_MODER_MODER9_1; //Set alternate function
  GPIOA->AFR[1] |= 7<<4;               //Set alternate function 7 (UART)
  
  // RX
  GPIOA->MODER &= ~GPIO_MODER_MODER10;  //Clear MODER
  GPIOA->MODER |= GPIO_MODER_MODER10_1; //Set alternate function
  GPIOA->AFR[1] |= 7<<8;               //Set alternate function 7 (UART)
  
  //Important!!! no floating inputs TX  pull high and RX pull low
  GPIOA->PUPDR |= GPIO_PUPDR_PUPDR9_0 | GPIO_PUPDR_PUPDR10_1;
  
  NVIC_EnableIRQ(USART1_IRQn); // CM4 Intrinsic
  USART1->CR1 |= USART_CR1_RXNEIE;
}
//-----------------------------------------------------------------------UartISR
void HAL::UartISR(uint8_t idx)
{
  //sysPtr->Uart[idx].ISR();
  switch(idx)
  {
  case 1:
    sysPtr->uart1.ISR();
    break;
  case 2:
    sysPtr->uart2.ISR();
    break;
  case 3:
    sysPtr->debugUart.ISR();
    break;
  }
}
//--------------------------------------------------------oneMilliSecondSnapshot
static uint32_t masterCount = 0;
uint32_t HAL::OneMilliSecondSnapshot() // for Stopwatch use
{
  return masterCount;
}

extern "C"
{
#pragma call_graph_root="interrupt"
  void SysTick_Handler(void)
  {
    masterCount++;
  }
  //----------------------------------------------------------------------------
#pragma call_graph_root="interrupt"
  void USART1_IRQHandler(void)
  {
    HAL::UartISR(1);
    NVIC_ClearPendingIRQ(USART1_IRQn); // Clear whatever invoked this
  }
  //----------------------------------------------------------------------------
#pragma call_graph_root="interrupt"
  void USART2_IRQHandler(void)
  {
    HAL::UartISR(2);
    NVIC_ClearPendingIRQ(USART2_IRQn); // Clear whatever invoked this
  }
#pragma call_graph_root="interrupt"
  void USART3_IRQHandler(void)
  {
    HAL::UartISR(3);
    NVIC_ClearPendingIRQ(USART3_IRQn); // Clear whatever invoked this
  }
}