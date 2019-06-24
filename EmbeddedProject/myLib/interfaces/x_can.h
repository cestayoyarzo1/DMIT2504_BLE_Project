/*
  FILE: x_can.h
  Created on: 10/18/2016, by Tom Diaz
*/

#ifndef X_CAN_H
#define X_CAN_H

static const char* CanBaudRateStr[] =
{
  "250k",
  "500k",
  "Auto"
};
//-------------------------------------------------------------------CanBaudRate
class CanBaudRate
{
public:
  enum Value
  {
    B_250k,
    B_500k,
    Auto
  };
};
//----------------------------------------------------------------------CanState
class CanState
{
public:
  enum Value
  {
    Sleep,
    Initialization,
    Normal
  };
};
//------------------------------------------------------------------------CANMsg
class CANMsg
{
public:
  CANMsg& operator = (CANMsg src);
  CANMsg& operator = (CANMsg* src);
  void Clear();
  uint32_t Id;
  uint8_t Data[8];
  uint8_t Length;
  uint8_t Format;
  uint8_t Type;
  
};

class x_CAN
{
public:
  x_CAN();
  ~x_CAN(){};
  virtual CanState::Value FSM() = 0;
  virtual void ShowAct() = 0;
  bool Push(CANMsg *msg);
  bool Pop(CANMsg* msg);
  void SetBaudRate(CanBaudRate::Value br);
  CanBaudRate::Value GetBaudRate();
protected:
  CANMsg list[CAN_MESSAGE_LIST_SIZE];
  uint8_t head;
  uint8_t tail;
  Stopwatch fsmTimer;
  CanState::Value state;
  CanState::Value prevState;
  CanBaudRate::Value baudrate;
private:
};


#endif /* X_CAN_H */
