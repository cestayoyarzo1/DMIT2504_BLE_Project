#include <hal.h>
#define CAN_SILENT false

//------------------------------------------------------------------------------
BCAN::BCAN()
{
  head = 0;
  tail = 0;

}
//------------------------------------------------------------------------------
/*bool BCAN::Connected()
{
  return fsmState == NormalMode;
}*/
//------------------------------------------------------------------------------
CanState::Value BCAN::FSM()
{
  switch(fsmState)
  {
  case SleepMode:
    state = CanState::Sleep;
    break;
  case WaitingForInit:
    if(can->MSR & CAN_MSR_INAK)
    {
      state = CanState::Initialization;
      fsmState = WaitingForNormal;
      can->MCR &= ~CAN_MCR_INRQ; //Request normal mode
      fsmTimer.Reset();
    }
    else if(fsmTimer.Read()>=100)
      Sleep();
    break;
  case WaitingForNormal:
    if(!(can->MSR & CAN_MSR_INAK))
    {
      fsmState = NormalMode;
      fsmTimer.StopAndReset();
    }
    else if(fsmTimer.Read()>=100)
      Sleep();
    break;
  case NormalMode:
    state = CanState::Normal;
    //canACT.Clear();
    break;
  }
  return state;

}
void BCAN::Filter()
{
  /*
    //----Filter configuration taken from CUBEMX generated code
  //-----No idea what it does (Yet)
  can->FMR |= 1;
  can->FA1R &= ~1;
  can->FS1R |= 1;
  */
  
  /*
  can->FM1R &= ~1;
  can->FFA1R &= ~1;
  can->FA1R |= 1;
  can->FMR &= ~1;
  //-------------Filter stuff end
  */
  can->FMR |= CAN_FMR_FINIT;            //filter initialization mode
  
  can->FA1R &= ~CAN_FA1R_FACT;          //de-activate all filters
  
  for(uint8_t i=0;i<14;i++)             //reset filters(make all filter registers = 0)
  {
    can->sFilterRegister[i].FR1 = 0;
    can->sFilterRegister[i].FR2 = 0;
  }
  
  can->FM1R &= ~(CAN_FM1R_FBM0 | CAN_FM1R_FBM1);        //filter 0,1 in identifier mask mode
  can->FS1R |= CAN_FS1R_FSC0 | CAN_FS1R_FSC1;           //filter O,1 single 32bit scale configuration
  can->FFA1R &= ~(CAN_FFA1R_FFA0 | CAN_FFA1R_FFA1);     //filter 0,1 assigned to fifo 0
  
  can->sFilterRegister[0].FR1 = (0x00F00300)<<3;        //ID
  //can->sFilterRegister[0].FR2 = (0x00FFF800)<<3;        //mask , 
  can->sFilterRegister[0].FR2 = (0x00000000)<<3;        //mask , 
  can->sFilterRegister[1].FR1 = (0x00FE6C00)<<3;        //ID
  //can->sFilterRegister[1].FR2 = (0x00FF0000)<<3;        //MASK  
  can->sFilterRegister[0].FR2 = (0x00000000)<<3;        //mask ,
  
  
  // Filters for OBDII****
  //can->sFilterRegister[2].FR1 = (0x7E0)<<20;            //ID
  //can->sFilterRegister[2].FR2 = (0x7F0)<<20;            //MASK
  //OBD2*******
  
  can->FA1R |= CAN_FA1R_FACT0 | CAN_FA1R_FACT1;          //activate filter 0,1
  can->FMR &= ~CAN_FMR_FINIT;                           //filters active mode 
  
}

//------------------------------------------------------------------------------
void BCAN::Init(CAN_TypeDef* can_ , Stm32CanBR::Value bRate)
{
  can = can_;
  state = prevState = CanState::Sleep;
  
  can->MCR &= ~CAN_MCR_SLEEP;          //Clear sleep mode if enabled
  can->MCR |= CAN_MCR_INRQ;            //Request Initialization mode
  while(!(can->MSR & CAN_MSR_INAK));   //Wait until Init request is acknowledged
  
 
  switch(bRate)
  {
  case Stm32CanBR::B_250k:
    can->BTR = Stm32CanBR::B_250k;             //Set baudrate
    break;
  case Stm32CanBR::B_500k:
     can->BTR = Stm32CanBR::B_500k;             //Set baudrate
    break;
  default:
    can->BTR = Stm32CanBR::B_250k;             //Set baudrate
    break;
  }
  
  if(CAN_SILENT)
    can->BTR |= CAN_BTR_SILM;           //Silent mode
  //can->BTR |= CAN_BTR_LBKM;           //Loopback mode
  //can->MCR |= 1<<16;                    //Debug Freeze
  can->IER |= CAN_IER_FMPIE0; //| CAN_IER_FMPIE1; //Interrupt on message received
  
  Filter();     //Apply filters
  
  /*
  can->MCR |= CAN_MCR_SLEEP;           //Set Sleep mode
  can->MCR &= ~CAN_MCR_INRQ;           //Request sleep mode
  while(can->MSR & CAN_MSR_SLAK);      //Wait until Seep mode is ack
  */ 
  can->MCR &= ~CAN_MCR_INRQ;           //Request normal mode
  while(can->MSR & CAN_MSR_INAK);      //Wait until Normal mode is ack
  
}
//------------------------------------------------------------------------------
void BCAN::ForceNormal()
{
  can->MCR &= ~CAN_MCR_SLEEP;          //Clear sleep mode if enabled
  can->MCR |= CAN_MCR_INRQ;            //Request Initialization mode
  while(!(can->MSR & CAN_MSR_INAK));   //Wait until Init request is acknowledged
  
  can->MCR &= ~CAN_MCR_INRQ;           //Request normal mode
  while(can->MSR & CAN_MSR_INAK);      //Wait until Normal mode is ack
}
//------------------------------------------------------------------------------
void BCAN::WakeUp()
{
  can->MCR &= ~CAN_MCR_SLEEP;
  can->MCR |= CAN_MCR_INRQ;
  fsmState = WaitingForInit;
  if(autoSleep)
    fsmTimer.Start();

}
//------------------------------------------------------------------------------
void BCAN::Sleep()
{
  
  can->MCR |= CAN_MCR_SLEEP;
  can->MCR &= ~CAN_MCR_INRQ;
  fsmTimer.StopAndReset();
  fsmState = SleepMode;

}
//------------------------------------------------------------------------------

void BCAN::ISR(CAN_FIFOMailBox_TypeDef* mailBox, volatile uint32_t* RFxR)
{
  uint8_t pending = *RFxR & CAN_RF0R_FMP0;//at least 1 message is pending
  
  while(pending)        //message pending in the receive FIFO
  {
    CANMsg msg; 
    msg.Clear();             
    *RFxR |= CAN_RF0R_RFOM0;            // Release message
    
    msg.Format = (mailBox->RIR & CAN_RI0R_IDE)>>2;//format 0->Standard Idetifier, format 1->extended identifier
    if(msg.Format)                     // Extended Identifier (1)
    {
      msg.Id = (mailBox->RIR & CAN_RI0R_EXID)>>3;
      msg.Id += (mailBox->RIR & CAN_RI0R_STID)>>3;
    }
    else                               // Standard ID
      msg.Id += (mailBox->RIR & CAN_RI0R_STID)>>21;
    
    msg.Length = mailBox->RDTR & CAN_RDT0R_DLC;
    msg.Type = mailBox->RIR & CAN_RI0R_RTR;
    
    memcpy(msg.Data,(void const *)(&mailBox->RDLR),8);
    Push(&msg);
    pending = *RFxR & CAN_RF0R_FMP0;
  }
  *RFxR &= ~(CAN_RF0R_FULL0 | CAN_RF0R_FOVR0); //Clear full and overrun
}
//------------------------------------------------------------------------------
void BCAN::Send(CAN_TxMailBox_TypeDef* msg) 
{
  
}

void BCAN::TxSetup(CAN_TxMailBox_TypeDef* mailbox, uint32_t ID, uint8_t IDE)
{
 // 
  //mailbox->
 // if(IDE)
   // mailbox->
}

void BCAN::ShowAct()
{
  //canACT.Toggle();
}
