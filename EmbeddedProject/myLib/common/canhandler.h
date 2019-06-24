/*
  FILE: canhandler.h
  Created on: 10/11/2016, by Tom Diaz
*/

#ifndef CANHANDLER_H
#define CANHANDLER_H

#include "x_can.h"

class CanHandler : public x_Handler
{
  public:
    CanHandler();
    void AddCan(x_CAN* canCtrler);
    void ParseCommand(CANMsg* msg);
    void ExtRpm(CANMsg* msg);
    void ExtCoolTemp(CANMsg* msg);
    void ExtAccPedPos1(CANMsg* msg);
    void ExtTachSpeed(CANMsg* msg);
    void fsm();
    uint16_t ReadRpm();
  private:
    x_CAN* can[NUMBER_OF_CAN];
    CANMsg* tmp;
    uint16_t rpm;     //PGN 0xF004, SPN 190, priority byte 0x0C   
    int16_t coolTemp;   //PGN 0xFEEE, SPN 110, priority byte 0x18
    uint16_t man1Press; //PGN 0xFEF6, SPN 81, priority byte 0x18
    uint16_t tachSpeed; //PGN 0xFEF6, SPN 1624, priority byte 0x0C
    uint8_t accPedPos1; //PGN 0xF003, SPN 91, priority byte 0x0C
    uint8_t activeCan;
    uint32_t wdg;
};

#endif /* CANHANDLER_H */