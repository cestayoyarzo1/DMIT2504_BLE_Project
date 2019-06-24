/*
 * uart.cpp
 *
 *  Created on: Sep 15, 2016
 *      Author: Tom Diaz
 */

#include <hal.h>	// Platform dependent
#include <uart.h>

//------------------------------------------------------------------------------
void UART::Init(COMId::Value uid, BufferHandler* rx, BufferHandler* tx, 
                bool dmaEnabled_)
{
  Init(uid,BaudRate::B_115200, rx, tx, dmaEnabled_);
}
//------------------------------------------------------------------------------
void UART::Init(COMId::Value uid, BaudRate::Value baudRate_, BufferHandler* rx, 
                BufferHandler* tx, bool dmaEnabled_)
{
  id = uid;
  
  uart = (USART_TypeDef *)uid;
  RxBuffer = rx;
  TxBuffer = tx;
  //TxBuffer.Init(txb,COM_PORT_BUFFER_SIZE);
  //RxBuffer.Init(rxb,COM_PORT_BUFFER_SIZE);
  rxbuffer = RxBuffer->GetPointer(0);
  cmdComplete=false;
  mode = CommandMode::NewLine;
  dmaEnabled = dmaEnabled_;
  baudRate = baudRate_;
  // Disable UART
  uart->CR1 &= ~(USART_CR1_UE | USART_CR1_TE | USART_CR1_RE);
  uart->BRR = getBRR(baudRate);
  // Enable UART, Tx and Rx
  uart->CR1 |= USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;
}
//------------------------------------------------------------------------------
uint32_t UART::getBRR(uint32_t baudRate)
{
  uint32_t brr = 0;

  brr = HAL::SysClk() / baudRate;

  return brr;
}
//------------------------------------------------------------------------------
void UART::AddToRx(uint8_t byte)
{
  tOut.isRunning()?tOut.Reset():tOut.Start();

  if(mode==CommandMode::NewLine && byte=='\r')
    cmdComplete=true;
  else
  {
    RxBuffer->Add<uint8_t>(byte);
  }
}
//------------------------------------------------------------------------------
void UART::SetParity(UartParity::Value p)
{
  uart->CR1 &= ~USART_CR1_UE; // Disable UART
  switch(p)
  {
  case UartParity::None:
    uart->CR1 &= ~USART_CR1_PCE; // Disable parity
    break;
  case UartParity::Even:
    uart->CR1 |= USART_CR1_M; // Enable 9 bit char (1 extra parity bit)
    uart->CR1 |= USART_CR1_PCE; // Enable parity
    uart->CR1 &= ~USART_CR1_PS; // Enable parity even
    break;
  case UartParity::Odd:
    uart->CR1 |= USART_CR1_M; // Enable 9 bit char (1 extra parity bit)
    uart->CR1 |= USART_CR1_PCE; // Enable parity
    uart->CR1 |= USART_CR1_PS; // Enable parity odd
    break;
  }
  uart->CR1 |= USART_CR1_UE; // Enable UART
}
//------------------------------------------------------------------------------
void UART::WriteByte(uint8_t byte)
{
  while(!(uart->ISR&USART_ISR_TXE));
  //while(uart->ISR & USART_ISR_BUSY);
  uart->TDR = byte;
  //while(!(uart->ISR&USART_ISR_TXE));    //wait fo data to be transfered to the shift reg.
}
//------------------------------------------------------------------------------
void UART::writeString(uint8_t* str, uint16_t length)
{
  for(uint16_t i=0;i<length;i++)
    WriteByte(str[i]);
  /*for(uint16_t i=length;i>0;i--)
    WriteByte(str[i]);*/
}
//------------------------------------------------------------------------------
void UART::FlushTx()
{
  writeString(TxBuffer->GetPointer(0),TxBuffer->Length());
  TxBuffer->Clear();
}
//------------------------------------------------------------------------------
void UART::ISR()
{
  uint32_t IIR = uart->ISR;
  
  /*if(IIR & USART_ISR_FE)
    uart->ICR |= USART_ICR_FECF;
  else if(IIR & USART_ISR_ORE)
    uart->ICR |= USART_ICR_ORECF;*/
  
  if((IIR & USART_ISR_FE) || (IIR & USART_ISR_ORE))
  {
    
    uart->ICR |= USART_ICR_FECF;
    uart->ICR |= USART_ICR_ORECF;
    uint8_t ch = uart->RDR;
  }
  else if(IIR & USART_ISR_RXNE)      //Rx
  {
    uint8_t ch = uart->RDR;
    AddToRx(ch);
  }
}
