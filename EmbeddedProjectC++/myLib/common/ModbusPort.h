/*
* ModbusPort.h
*
*  Created on: Jan 26, 2017
*      Author: E1216431
*/

#ifndef MODBUSPORT_H_
#define MODBUSPORT_H_

#define MODBUS_BUFFER_SIZE 128

typedef struct
{
    uint16_t address; ///< The modbus address of this item.
    uint16_t(*readFunction)(void*, int16_t); ///< The read function at this address. Can be NULL if this is a write-only register. The arg is passed as the first parameter.
    uint16_t(*writeFunction)(void*, int16_t); ///< The write function at this address. Can be NULL if this is a read-only register. The arg is passed as the first parameter.
    void* arg; ///< A pointer passed in to the read and write functions. Be careful to ensure the function is expecting the correct type.
} ModbusFunctionItem;

class ModbusPort
{
public:
  class Function
  {
  public:
    enum Value
    {
      Read_Coil=1,
      Read_Input=2,
      Read_Holding_Register=3,
      Read_Input_Register=4,
      Write_Single_Coil=5,
      Write_Single_Register=6,
      Write_Multiple_Coils=15,
      Write_Multiple_Registers=16,
    };
  };
  ModbusPort();
  virtual ~ModbusPort();
  void Init(COMId::Value uid, BaudRate::Value baudRate_);
  void ISR();
  void FSM();
  uint16_t Rev16(uint32_t address);
protected:
  uint32_t getBRR(uint32_t baudRate);
  uint32_t baudRate;
  uint16_t crc[2];
  USART_TypeDef *uart;
  CommandMode::Value mode;
  COMId::Value id;
  uint8_t address;
  bool cmdComplete;
  BufferHandler RxBuffer;
  BufferHandler TxBuffer;
  uint8_t txb[MODBUS_BUFFER_SIZE];
  uint8_t rxb[MODBUS_BUFFER_SIZE];
  
};

#endif /* MODBUSPORT_H_ */
