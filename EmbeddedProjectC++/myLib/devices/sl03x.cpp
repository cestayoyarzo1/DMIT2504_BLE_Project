/*
  FILE: sl03x.cpp
  Created on: 9/17/2016, by Tom Diaz
*/

#include <hal.h>
#include <sl03x.h>
#include <string.h>

#define FSM_TIMEOUT 500
#define MAX_AUTH_CARDS 2

#define STATUS nfcCmd[3]
#define DESFIRE_STATUS nfcCmd[4]
#define DESFIRE_PAYLOAD nfcCmd[5]
#define DATA_BYTES nfcCmd[1]

static const char authCards[MAX_AUTH_CARDS][7] = 
{
  /*{0x04,0x1A,0x2A,0xEA,0x75,0x31,0x80},*/
  {0x04,0x48,0x2F,0xEA,0x75,0x31,0x80},
  {0x04,0x75,0x3D,0xEA,0x75,0x31,0x80}
};

SL03x::SL03x()
{
  desfireFsmState = DesfireFsmState::Idle;
  desfireCmd[0] = DesfireCommand::None;
  desfireCmd[1] = DesfireCommand::None;
  lastStatus = SL03x::ResponseStatus::NoStatus;
  lastDesfireStatus = DesfireStatus::NoStatus;
  
  dataBuffer.Init(databuffer,NFC_DATA_BUFFER);
}
//------------------------------------------------------------------------------
void SL03x::clearActiveAID()
{
  memset(activeAID,0,4);
}
//------------------------------------------------------------------------------
void SL03x::setActiveAID(uint8_t* aid)
{
  memcpy(activeAID+1,aid,3);
}
//------------------------------------------------------------------------------
void SL03x::validateActiveAID()
{
  activeAID[0] = true;
}
//------------------------------------------------------------------------------
bool SL03x::compareActiveIAD(uint8_t* aid)
{
  return activeAID[0]?!memcmp(activeAID+1,aid,3):false;
}
//------------------------------------------------------------------------------
void SL03x::fsm()
{  
  ResetFsmTask();
  
  switch(fsmState)
  {
  case FsmState::SelectCard:
    SendCommand(Command::SelectMifareCard);
    SetFsmState(FsmState::WaitForAnswer);
    resetFsmTimeout();
    break;
  case FsmState::RequestforAnswer:
    SendCommand(Command::RequestForAnswer);
    SetFsmState(FsmState::WaitForAnswer);
    resetFsmTimeout();
    break;
  case FsmState::Locked:
    //Card is authenticated, PC app issued a lock to avoid losing
    //session by scanning for card again.
    resetFsmTimeout();
    break;
  case FsmState::Idle:
    //Card entered and stills in range, idle
    //state waits for commands (write, read)
    break;
  case FsmState::WaitForAnswer:
    //Wait for Response in UART buffer
    //ResetFsmTask handles timeout
    break;
  case FsmState::Error:
    lastStatus = (ResponseStatus::Value)STATUS;
    if(cards[0].GetType()==CardType::Desfire)
      lastDesfireStatus=(DesfireStatus::Value)DESFIRE_STATUS;
    else
      lastDesfireStatus=DesfireStatus::NoStatus;
  case FsmState::Timeout:
    SetFsmState(FsmState::SelectCard);
    break;
    //-------------------------------------------------Desfire states
  case FsmState::ExchangeTransparentData:
    SendCommand(Command::ExchangeTransparentData,&dataBuffer);
    SetFsmState(FsmState::WaitForAnswer);
    resetFsmTimeout();
    break;
  case FsmState::TmpState:
    break;
  }
}

//------------------------------------------------------------------------------
bool SL03x::IsAuthenticated()
{
  return authenticated;
}
//------------------------------------------------------------------------------
void SL03x::SetFsmState(FsmState::Value s)
{
  fsmStatePast = fsmState;
  fsmState = s;
}
//------------------------------------------------------------------------------
void SL03x::Parse(COMPort* p)
{
  Command::Value cmd = (Command::Value)nfcCmd[2];//gather cmd for nfc reader
  switch(cmd)
  {
  case Command::SelectMifareCard: serviceSelectMifare(); break;/////////////////I am working here
  case Command::RequestForAnswer: serviceRequestForAnswer(); break;
  case Command::ExchangeTransparentData: serviceExchangeTransparentData(); break;
  case Command::SetUart:
    nfcUart = p;
    TxB = p->TxB();
    RxB = p->RxB();
    nfcCmd = p->RxB()->GetPointer(0);
    break;
  default:
	  break;
  }
  RxB->Clear();
}
//------------------------------------------------------------------------------
void SL03x::resetFsmTimeout()
{
  //nfcSchedule[Tasks::ResetFSM]=0;
  fsmTimeout = 0;
}
//------------------------------------------------------------------------------
void SL03x::ResetFsmTask()
{
  /*if(IsTaskDue(Tasks::ResetFSM,FSM_TIMEOUT))
    SetFsmState(FsmState::Timeout);*/
  if(fsmTimeout >= FSM_TIMEOUT)
  {
    fsmTimeout = 0;
    SetFsmState(FsmState::Timeout);
  }
  else
    ++fsmTimeout;
}
//------------------------------------------------------------------------------
/*void SL03x::ResetTaskCounter(Tasks::Value task)
{
  nfcSchedule[task] = 0;
}*/
//------------------------------------------------------------------------------
void SL03x::serviceSelectMifare()
{
	if (STATUS==0x01) // No Tag found
	{
		sys->Callback(CallbackMsg::Sys_Operate_Led,LedPattern::Beacon);

		cards[1] = cards[0]; //Backup last tag in range

		if(~cards[0])//Delete tag that just went out of range
		{
			//ReportEvent(&cards[1],false);
		  sys->Callback(CallbackMsg::NFC_Card_Out_Of_Range);
		}
		authenticated = false;

		SetFsmState(FsmState::SelectCard);
	}
	else if(STATUS==0) //Tag found
	{
		if(cards[0].Init(&nfcCmd[0]) || !authenticated) //If Tag wasn't already initialized
		{
			sys->Callback(CallbackMsg::Sys_Operate_Led,LedPattern::Toggle_25ms);
			SetFsmState(FsmState::RequestforAnswer); //Start tag initialization

			if(cards[0].GetType()!=CardType::Desfire)
			  sys->Callback(CallbackMsg::NFC_Card_In_Range);
		}
		/*else //Tag was already initialized so go to Idle state
		 {
		 SetFsmState(FsmState::Idle);
		 }*/
	}
	else
	{
		SetFsmState(FsmState::Error);
	}

}
//------------------------------------------------------------------------------
void SL03x::serviceRequestForAnswer()
{
  if(STATUS==0x00)
  {
    switch(cards[0].GetType())
    {
    case CardType::Desfire:
      SetDesfireCommand(DesfireCommand::GetVersion);
      SetFsmState(FsmState::ExchangeTransparentData);
      break;
    default:
      SetFsmState(FsmState::SelectCard);
      break;
    }
  }
  else
  {
    SetFsmState(FsmState::Error);
  }
}
//------------------------------------------------------------------------------
void SL03x::serviceExchangeTransparentData()
{
  //----------------Serves only DESFire Cards for now
  if(STATUS==0)
  {
    switch(desfireCmd[0])
    {
    case DesfireCommand::GetVersion: serviceGetDesfireVersion(); break;
    case DesfireCommand::GetApplicationIDs: serviceGetDesfireAppIDs(); break;
    case DesfireCommand::Authenticate: serviceDesfireAuthenticate(); break;
    default:
    	break;
    }
  }
  else
  {
    SetFsmState(FsmState::Error);
  }
}
//------------------------------------------------------------------------------
void SL03x::SetDesfireCommand(DesfireCommand::Value cmd)
{
  if((cmd == DesfireCommand::AdditionalFrame)
     && (desfireCmd[1] != DesfireCommand::AdditionalFrame))
  {
    desfireCmd[1] = desfireCmd[0];
  }
  else
    desfireCmd[0] = cmd;

  dataBuffer.Clear();
  dataBuffer.AddByte(cmd);
}
//------------------------------------------------------------------------------
void SL03x::serviceGetDesfireVersion()
{
  switch((DesfireStatus::Value)DESFIRE_STATUS)
  {
  case DesfireStatus::AdditionalFrame:
      SetDesfireCommand(DesfireCommand::AdditionalFrame);
      SetFsmState(FsmState::ExchangeTransparentData);
      cards[0].AddExtraData(nfcCmd);
      break;
  case DesfireStatus::OperationOK:
    cards[0].AddExtraData(nfcCmd);
    SetDesfireCommand(DesfireCommand::Authenticate);    
    dataBuffer.AddByte(0); // Key 0 (default key)
    SetFsmState(FsmState::ExchangeTransparentData);
    break;
  default:
    SetFsmState(FsmState::Error);
    break;
  }
}
//------------------------------------------------------------------------------
void SL03x::serviceGetDesfireAppIDs()
{
  //STM32F042xx* s  = (STM32F042xx*)sys;
  switch((DesfireStatus::Value)DESFIRE_STATUS)
  {
  case DesfireStatus::OperationOK:
    extractApplicationIds(true);
    /*if(s->ConnectionHandler.Connected() &&
       (s->ConnectionHandler.PHY() == ConnectionSource::STLink))
      SetFsmState(FsmState::Locked);
    else
      SetFsmState(FsmState::Idle);*/
    break;
  case DesfireStatus::AdditionalFrame:
    extractApplicationIds(false);
    SetDesfireCommand(DesfireCommand::AdditionalFrame);
    SetFsmState(FsmState::ExchangeTransparentData);
    break;
  default:
    SetFsmState(FsmState::Error);
    break;
  }
}
//------------------------------------------------------------------------------
void SL03x::serviceDesfireAuthenticate()
{
  switch ((DesfireStatus::Value) DESFIRE_STATUS)
  {
    case DesfireStatus::AdditionalFrame:
    //-------------------------------------------
    SetDesfireCommand(DesfireCommand::AdditionalFrame);
    //Datasheet is wrong, this is correct: 3DES(randA) + 3DES(randB' XOR 3DES(randA))
    Des.Decrypt(RxB->GetPointer(5));
    dataBuffer.AddArray((uint8_t*)"olakasee",8);
    RxB->RotateLeft(5,8);
    RxB->Xor(5,dataBuffer.GetPointer(1),8);
    Des.Decrypt(RxB->GetPointer(5));
    dataBuffer.AddArray(RxB->GetPointer(5),8);
    SetFsmState(FsmState::ExchangeTransparentData);
    break;
    case DesfireStatus::OperationOK:
    authenticated = true;
    sys->Callback(CallbackMsg::Sys_Operate_Led,LedPattern::On);
    sys->Callback(CallbackMsg::NFC_Desfire_Authenticated);
    SetDesfireCommand(DesfireCommand::GetApplicationIDs);
    SetFsmState(FsmState::ExchangeTransparentData);
    break;
    default:
    SetFsmState(FsmState::Error);
    break;
  }
}
//------------------------------------------------------------------------------
//                      DESFire Methods [END]
//------------------------------------------------------------------------------
void SL03x::extractApplicationIds(bool end)
{
  //STM32F042xx* s  = (STM32F042xx*)sys;
  //Discard SL03x Len, Command and Chk and DesfireStatus
  uint32_t dataBytes = DATA_BYTES-4;
  uint8_t apps = dataBytes/3;
  if(dataBytes>2) // Answer is one or more 3-Byte Applications ID
  {
    if(apps)
    {
      /*
      s->CommUart->TxBuffer.Clear();
      s->CommUart->TxBuffer.AddArray((void*)"@1");
      s->CommUart->TxBuffer.AddByte(3); //3 means IDs payload
      s->CommUart->TxBuffer.AddByte(end);
      s->CommUart->TxBuffer.AddArray(&DESFIRE_PAYLOAD,dataBytes);
      s->CommUart->TxBuffer.ToB64();
      s->CommUart->TxBuffer.AddCr();
      s->CommUart->FlushTx();
      */
    }
  }
}
//------------------------------------------------------------------------------
void SL03x::SendCommand(Command::Value cmd)
{
  TxB->Clear();
  TxB->AddByte(0xBA);
  TxB->AddByte(0);
  TxB->AddByte(cmd);
  TxB->PackForSL03x();
  nfcUart->FlushTx();
}
//------------------------------------------------------------------------------
void SL03x::SendCommand(Command::Value cmd,BufferHandler* buffer)
{
  TxB->Clear();
  TxB->AddByte(0xBA);
  TxB->AddByte(0);
  TxB->AddByte(cmd);
  TxB->AddArray(buffer->GetPointer(0),buffer->Length());
  TxB->PackForSL03x();
  nfcUart->FlushTx();
}
//------------------------------------------------------------------------------
void SL03x::CreateAID(uint8_t* aid,uint8_t settings,uint8_t nKeys)
{
  SetDesfireCommand(DesfireCommand::CreateApplication);
  dataBuffer.AddArray(aid,3);
  dataBuffer.AddByte(settings);
  dataBuffer.AddByte(nKeys);
  SetFsmState(FsmState::ExchangeTransparentData);
}
//------------------------------------------------------------------------------
void SL03x::RestartScanning()
{
  fsmState = FsmState::Timeout;
}
//------------------------------------------------------------------------------
/*bool SL03x::IsTaskDue(Tasks::Value task,uint32_t value)
{
  if(nfcSchedule[task]>=value)
  {
    nfcSchedule[task]=0;
    return true;
  }
  else
  {
    nfcSchedule[task]+=1;
    return false;
  }
}*/
