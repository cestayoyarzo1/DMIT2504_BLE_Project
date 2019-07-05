/*
FILE: sl03x.h
Created on: 9/17/2016, by Tom Diaz
*/

#ifndef SL03X_H
#define SL03X_H

#define SL03X_TASKS 1

//-------------------------------------------------------------------------SL03x
class SL03x : public x_Handler
{
public:
  //------------------------------------------------Class enclosed enums [Begin]
  class FsmState
  {
  public:
    enum Value {SelectCard=0,RequestforAnswer=1,ExchangeTransparentData=0x03,
    Locked=0x04,Idle=0xfb,TmpState=0xfc,WaitForAnswer=0xfd,Error=0xfe,Timeout=0xff};
  };
  class Tasks
  {
  public:
    enum Value { ResetFSM };
  };
  class Command
  {
  public:
    enum Value { SetUart = 0x00, SelectMifareCard = 0x01,LoginToASector = 0x02,ReadDataBlock = 0x03,
      WriteDataBlock = 0x04,ReadValueBlock = 0x05,InitValueBlock = 0x06,
      WriteMasterKeyA = 0x07,IncrementValue = 0x08,DecrementValue = 0x09,
      CopyValue = 0x0A,ReadDataPage = 0x10,WriteDataPage = 0x11,DownloadKey = 0x12,
      LoginSectorViaStoredKey = 0x13,RequestForAnswer = 0x20,
      ExchangeTransparentData = 0x21,ManageLed = 0x40,PowerDown = 0x50,
      GetFirmwareVersion = 0xF0,NoCommand = 0xFF };
  };
  class ResponseStatus
  {
  public:
    enum Value {OperationSucceded = 0x00,NoTag = 0x01,LoginSuccess = 0x02,
    LoginFailed = 0x03,ReadFailed = 0x04,WriteFailed = 0x05,
    UnableToReadAfterWrite = 0x06,AddressOverflow = 0x08,ATSFailed = 0x10,
    CommunicationFailed = 0x11,CollisionOccurred = 0x0A,NotAuthenticated = 0x0D,
    NotAValueBlock = 0x0E,ChecksumError = 0x0F,CommandCodeError = 0xF1,NoStatus=0xff,
    };
  };
  //--------------------------------------------------Class enclosed enums [End]
  SL03x();
  ~SL03x(){};
  void SendCommand(Command::Value cmd);
  void SendCommand(Command::Value cmd,BufferHandler* buffer);
  void Parse(COMPort* p);
  //void Init(void* system_, UART* nfcUart_);
  bool IsAuthenticated();
  void extractApplicationIds(bool end);
  void CreateAID(uint8_t* aid,uint8_t settings,uint8_t nKeys);
  void RestartScanning();
private:
  /*class CallbackType
  {
  public:
    enum Value
    {
      InRangeChanged,
    };
  };*/
  void fsm();
  void clearActiveAID();
  void setActiveAID(uint8_t* aid);
  bool compareActiveIAD(uint8_t* aid);
  void validateActiveAID();
  void SetDesfireCommand(DesfireCommand::Value cmd);
  //void SetLed(LedPattern::Value pattern_);
  //void RxTask();
  void resetFsmTimeout();
  void ResetFsmTask();
  //bool IsTaskDue(Tasks::Value task,uint32_t value);
  void serviceSelectMifare();
  void serviceRequestForAnswer();
  //void callback(CallbackType::Value c);
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Desfire Engine BEGIN
  //void desfireFSM();
  //void serviceAdditionalFrame();
  void serviceGetDesfireVersion();
  void serviceGetDesfireAppIDs();
  void serviceDesfireAuthenticate();
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Desfire Engine END
  void serviceExchangeTransparentData();
  void getUID();
  //void ResetTaskCounter(Tasks::Value task);
  void SetFsmState(FsmState::Value s);
  ResponseStatus::Value LastError;
  //-------------------------------------------------Vars
  //uint8_t appIdTmp;
  COMPort* nfcUart;
  GPIO* reset;
  BufferHandler* TxB;
  BufferHandler* RxB;
  uint8_t* nfcCmd;
  BufferHandler dataBuffer;
  uint8_t databuffer[NFC_DATA_BUFFER];
  //uint32_t nfcSchedule[SL03X_TASKS];
  uint16_t fsmTimeout;
  NfcCard cards[2];
  FsmState::Value fsmStatePast;
  SL03x::FsmState::Value fsmState;
  DesfireFsmState::Value desfireFsmState;
  DesfireCommand::Value desfireCmd[2];
  SL03x::ResponseStatus::Value lastStatus;
  DesfireStatus::Value lastDesfireStatus;
  bool authenticated;
  //uint32_t id;
  static uint32_t staticId;
  DES Des;
  uint8_t activeAID[4];
};

#endif /* SL03X_H */
