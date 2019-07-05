/*
  FILE: bcan.h
  Created on: 10/18/2016, by Tom Diaz
*/

#ifndef BCAN_H
#define BCAN_H

#include "x_can.h"

//-------------------------------------------------------------------------CanBR
class Stm32CanBR
{
public:
  enum Value
  {
    //B_250k = 0x001c000b,//0x001c0013,
    B_250k = 0x001c0013,        
    //B_500k = 0x001c0005,//0x001c0009,
    B_500k = 0x001c0009,
    //B_1000k = 0x001c0002//0x001c0004,
    B_1000k = 0x001c0004,
  };
};

class BCAN :    public x_CAN
{
public:
  BCAN();
  void Init(CAN_TypeDef* can_ , Stm32CanBR::Value bRate); 
  void ISR(CAN_FIFOMailBox_TypeDef* mailBox, volatile uint32_t* RFxR);
  void WakeUp();
  void Sleep();
  void Normal();
  void ForceNormal();
  CanState::Value FSM();
  void Filter();
  void Send(CAN_TxMailBox_TypeDef* );
  void TxSetup(CAN_TxMailBox_TypeDef* mailbox, uint32_t ID, uint8_t IDE);
  void ShowAct();
  CAN_TypeDef* can;
private:
  //GPIO canACT;
  enum FsmState
  {
    SleepMode,
    WaitingForInit,
    WaitingForNormal,
    NormalMode,
    WaitingForSleep
  };
  FsmState fsmState;
  bool autoSleep;

};

#endif /* BCAN_H */
