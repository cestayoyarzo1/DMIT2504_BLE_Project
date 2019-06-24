/*
  FILE: SysL476xx.cpp
  Created on: 8/13/2017, by Tom Diaz
*/

#include <hal.h>
#include <sysl476xx.h>
#include <stdio.h>

SysL476xx::SysL476xx()
{
  HAL::Init(this);
}
//------------------------------------------------------------------------------
void SysL476xx::Init()
{
  button.InitAsInput(GPIOC,13,Resistor::Pullup);
  buttonEx.Init(&button,false);
  led.InitAsOutput(GPIOA,5,false);
  ledEx.Init(&led,false);
  ledEx.SetPattern(LedPattern::Toggle_500ms);
  wifiBuffer.Init(wifiRawBuffer,COM_RX_BUFFER_SIZE);
  
  wifiRxBuffer.Init(wifiRawRxBuffer,COM_RX_BUFFER_SIZE);
  wifiTxBuffer.Init(wifiRawTxBuffer,COM_TX_BUFFER_SIZE);
  uart1.Init(COMId::COM_1,BaudRate::B_115200,&wifiRxBuffer,&wifiTxBuffer,false);
  uart1.SetCommandMode(CommandMode::Timeout);
  
  commRxBuffer.Init(commRawRxBuffer,COM_RX_BUFFER_SIZE);
  commTxBuffer.Init(commRawTxBuffer,COM_TX_BUFFER_SIZE);
  uart2.Init(COMId::COM_2,BaudRate::B_115200,&commRxBuffer,&commTxBuffer,false);
  uart2.SetCommandMode(CommandMode::Timeout);
  
  debugRxBuffer.Init(debugRawRxBuffer,COM_RX_BUFFER_SIZE);
  debugTxBuffer.Init(debugRawTxBuffer,COM_TX_BUFFER_SIZE);
  debugUart.Init(COMId::COM_3,BaudRate::B_115200,&debugRxBuffer,&debugTxBuffer,false);
  debugUart.SetCommandMode(CommandMode::Timeout);
  
  rtc.Init(RTC,PWR);
  timeHandler.Init(this,true);
  timeHandler.SetSource(&rtc);
  timeHandler.SetDestination(&time);
  rtc.x_RTC::Sync(946684800);
  
  comHandler.Add(&uart1);
  comHandler.Add(&uart2);
  comHandler.Add(&debugUart);
  comHandler.Init(this,true);
  paceTimer.Start();
  
  robot.Init(TIM3, TIM4);
  
  btSpi.Init(SPI1);
  btCs.InitAsOutput(GPIOA,1, true);
  leBluetooth.Init(&btSpi, &btCs, SpiClk::f_625K); 
  
}
//------------------------------------------------------------------------------
void SysL476xx::Run()
{
  for(;EVER;)
  {
    if(paceTimer.Read()>=1)
    {
      ledEx.FSM();
      comHandler.FSM(1); 
      timeHandler.FSM(1);
      robot.Run();
      
      switch(buttonEx.FSM())
      {
      case ButtonState::Pressed:
        break;
        
      case ButtonState::Held_First_Holding_Time:
        break;
        
      case ButtonState::Held_Second_Holding_Time:
        break;
        
      case ButtonState::Held_Fourth_Holding_Time:
        break;
      }
      
      paceTimer.Reset();
    }
  }
}
//-------------------------------------------------------------------------parse
void SysL476xx::parse(COMPort* p, uint16_t idx)
{
  uint8_t* cmd = p->RxB()->GetPointer(0);
}
//************************ Callback interface **********************************
//-----------------------------------------------------------------------Process
intptr_t SysL476xx::Process(uint32_t msg)
{
  intptr_t ans = 0;
  while(lock[0]){/* Trap, recursion incurred or lock not released */}
  lock[0] = true;
  
  switch(msg)
  {
  case Process::Parse_COMPort_Message:
    parse((COMPort*)Args[0],Args[1]);
    break;
  default:
    while(true){/* Undefined message */}
    break;
  }
  
  lock[0] = false;
  return ans;
}
//---------------------------------------------------------------------------Get
intptr_t SysL476xx::Get(uint32_t msg)
{
  intptr_t ans = 0;
  while(lock[1]){/* Trap, recursion incurred or lock not released */}
  lock[1] = true;
  
  switch(msg)
  {
  default:
    //while(true){/* Undefined message */}
    break;
  }
  
  lock[1] = false;
  return ans;
}
//---------------------------------------------------------------------------Set
intptr_t SysL476xx::Set(uint32_t msg)
{
  intptr_t ans = 0;
  while(lock[2]){/* Trap, recursion incurred or lock not released */}
  lock[2] = true;
  
  switch(msg)
  {
  case Set::Rtc:
    rtc.x_RTC::Sync((uint32_t)Args[0]);
    break;
  default:
    //while(true){/* Undefined message */}
    break;
  }
  
  lock[2] = false;
  return ans;
}
//-------------------------------------------------------------------------Clear
intptr_t SysL476xx::Clear(uint32_t msg)
{
  intptr_t ans = 0;
  while(lock[3]){/* Trap, recursion incurred or lock not released */}
  lock[3] = true;
  
  switch(msg)
  {
  default:
    //while(true){/* Undefined message */}
    break;
  }
  
  lock[3] = false;
  return ans;
}
