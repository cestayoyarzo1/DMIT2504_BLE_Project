/*
 * ModbusPort.cpp
 *
 *  Created on: Jan 26, 2017
 *      Author: E1216431
 */

#include <hal.h>
#include <ModbusPort.h>

ModbusPort::ModbusPort()
{
  address = 1;
}

ModbusPort::~ModbusPort()
{
	// TODO Auto-generated destructor stub
}

//------------------------------------------------------------------------------
void ModbusPort::Init(COMId::Value uid, BaudRate::Value baudRate_)
{
	id = uid;
	uart = (USART_TypeDef *) uid;
	TxBuffer.Init(txb, MODBUS_BUFFER_SIZE);
	RxBuffer.Init(rxb, MODBUS_BUFFER_SIZE);
	cmdComplete = false;
	baudRate = baudRate_;
	// Disable UART
	uart->CR1 &= ~(USART_CR1_UE | USART_CR1_TE | USART_CR1_RE);
	uart->BRR = getBRR(baudRate);
	uart->CR2 |= USART_CR2_RTOEN;	//Enable Rx Timeout
	uart->CR1 |= USART_CR1_RTOIE;	//Enable Tx Timeout interrupt
	uart->RTOR |= 44;				//44 bits timeout (4 chars)
	// Enable UART, Tx and Rx
	uart->CR1 |= USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;
}
//------------------------------------------------------------------------------
uint32_t ModbusPort::getBRR(uint32_t baudRate)
{
	uint32_t brr = 0;

	brr = HAL::SysClk() / baudRate;

	return brr;
}
//-----------------------------------------------------------------------------
void ModbusPort::ISR()
{
	uint32_t IIR = uart->ISR;

	if (IIR & USART_ISR_RXNE)      //Rx
	{
		uint8_t ch = uart->RDR;
		if (IIR & USART_ISR_ORE) // If Overrun error clear it
			uart->ICR |= USART_ICR_ORECF;
		else
			RxBuffer.Add<uint8_t>(ch);
	}
	if (IIR & USART_ISR_RTOF)	// Timeout used for mobdbus
	{
		uart->ICR |= USART_ICR_RTOCF;
		cmdComplete = true;
	}
}
//-----------------------------------------------------------------------------
void ModbusPort::FSM() // Call this once per millisecond
{
  if (cmdComplete)
  {
    cmdComplete = false;
    if (rxb[0] == address)  // If message is meant for me
    {
      if (RxBuffer.Length() > 3)     // Smallest modbus frame is 4 bytes
      {
        crc[0] = HAL::CRC16(RxBuffer.GetPointer(0), RxBuffer.Length() - 2,
            0xffff);
        crc[1] = RxBuffer.GetInt16(RxBuffer.Length() - 2);

        if (crc[0] == crc[1])
        {
          switch ((Function::Value)rxb[1])
          // Parse Function
          {
          case Function::Read_Coil:
            break;
          case Function::Read_Input:
            break;
          case Function::Read_Holding_Register:
            break;
          case Function::Read_Input_Register:
            break;
          case Function::Write_Single_Coil:
            break;
          case Function::Write_Single_Register:
            break;
          case Function::Write_Multiple_Coils:
            break;
          case Function::Write_Multiple_Registers:
            break;
          }
        }
      }
    }
    RxBuffer.Clear();
  }
}
//-----------------------------------------------------------------------------
uint16_t ModbusPort::Rev16(uint32_t address)
{
  return __REV16(RxBuffer.GetInt16(address));
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

