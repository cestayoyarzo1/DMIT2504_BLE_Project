/*
  FILE: x_can.cpp
  Created on: 10/18/2016, by Tom Diaz
*/
#include <hal.h>
#include <x_can.h>


//------------------------------------------------------------------------------
CANMsg& CANMsg::operator=(CANMsg* src)
{
  memcpy(this,src,sizeof(CANMsg));
  return *this;
}
//------------------------------------------------------------------------------
CANMsg& CANMsg::operator=(CANMsg src)
{
  memcpy(this,&src,sizeof(CANMsg));
  return *this;
}
//------------------------------------------------------------------------------
void CANMsg::Clear()
{
  memset(this,0,sizeof(CANMsg));
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
x_CAN::x_CAN()
{
  baudrate = CanBaudRate::B_250k;
}
//------------------------------------------------------------------------------
bool x_CAN::Pop(CANMsg* msg)
{
  msg->Clear();
  
  if(head == tail) return false;        //List is empty
  
  *msg = list[tail];
  list[tail].Clear();
  
  uint8_t next = tail+1;
  
  if(next >= CAN_MESSAGE_LIST_SIZE)
    next=0;
  
  tail = next;
  
  return true;
}
//------------------------------------------------------------------------------
bool x_CAN::Push(CANMsg *msg)
{
  uint8_t next = head+1;
  
  if(next >= CAN_MESSAGE_LIST_SIZE)
    next=0;
  
  if(next == tail) return false;        //Buffer is full
  
  list[head] = msg;
  head = next;
  
  return true;
}
//------------------------------------------------------------------------------
CanBaudRate::Value x_CAN::GetBaudRate()
{
  return baudrate;
}
//------------------------------------------------------------------------------
void x_CAN::SetBaudRate(CanBaudRate::Value br)
{
  baudrate = br;
}
//------------------------------------------------------------------------------
