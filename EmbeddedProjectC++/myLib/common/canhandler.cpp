#include <hal.h>

CanHandler::CanHandler()
{
  coolTemp=0;
  accPedPos1=0;
  man1Press=0;
  tachSpeed=0;
  activeCan=0;
  rpm = 0;
}

void CanHandler::AddCan(x_CAN* canCtrler)
{
  can[activeCan++]= canCtrler;  

}

void CanHandler::ParseCommand(CANMsg* msg)
{  
    int32_t pgn; 
    can[0]->ShowAct();
    pgn=(msg->Id&0x00FFFF00)>>8;
    tmp = msg;
    switch(pgn)
    {
    case 0xF003://PGN F003 , accelerator pedal %
      ExtAccPedPos1(msg);
      break;
    case 0xF004://PGN F004 , rpm // RPM ID
      ExtRpm(msg);
      break;
    case 0xFE6C://PGN FE6C , Tach Vehicle Speed Km/Hr
      ExtTachSpeed(msg);
      break;
 
    case 0xFEEE://PGN FEEE , Engine Coolant Temp
      ExtCoolTemp(msg);
      break;
    case 0XFEF6://PGN FEF6 , Engine Intake Manifold #1 Temp
      man1Press = msg->Data[1]*2;//pressure in kPa
      break;
    default:
      break;
    }
}
//------------------------------------------------------------------------------
void CanHandler::fsm()
{
  CANMsg msg;
  for(uint_fast8_t i = 0; i<activeCan; i++)
  {
    while(can[i]->Pop(&msg))
    {
      //callback->Set(Set::ToggleAuxLed);
      ParseCommand(&msg);
    }
  }
}
//------------------------------------------------------------------------------
void CanHandler::ExtRpm(CANMsg* msg)
{
  rpm = (uint16_t)(msg->Data[4])<<8;
  rpm = rpm | (uint16_t)(msg->Data[3]);
  rpm /=8;
}
//------------------------------------------------------------------------------
void CanHandler::ExtCoolTemp(CANMsg* msg)
{
  coolTemp = msg->Data[0]-40;
}
//------------------------------------------------------------------------------
void CanHandler::ExtAccPedPos1(CANMsg* msg)
{
  accPedPos1 = (uint8_t)(msg->Data[1]*0.4);  
}
//------------------------------------------------------------------------------
void CanHandler::ExtTachSpeed(CANMsg* msg)
{
  tachSpeed = msg->Data[7];
  tachSpeed <<=8;
  tachSpeed |= msg->Data[6];
  tachSpeed /=256;
}
//------------------------------------------------------------------------------
uint16_t CanHandler::ReadRpm()
{
  if(wdg>15)
  {
    wdg=16;
    rpm = 0;
  }
  else
    wdg+=1;
  return rpm;
}
//-----------------------------------------------------------------------------