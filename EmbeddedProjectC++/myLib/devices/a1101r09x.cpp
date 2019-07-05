/*
  FILE: a1101r09x.cpp
  Created on: 6/30/2017, by Tom Diaz
*/

#include <hal.h>
#include <a1101r09x.h>

static const uint8_t PATABLE = { 0xC0 };
//bool answers[0x2f];

//const uint8_t patable_power_915[] = {0x0B,0x1B,0x6D,0x67,0x50,0x85,0xC9,0xC1};

//------------------------------------------------------------------------------
void A1101R09x::Init(x_SPIPort* spi_, x_GPIO* cs_, x_GPIO* led_, x_Callback* cb_)
{
  spi = spi_;
  cs = cs_;
  cb = cb_;
  led.Init(led_,true);
  
  Buffer.Init(buffer,COM_PORT_RX_BUFFER_SIZE);
  pb.Init(pbuffer,PACKET_BUFFER_SIZE);
  
  reset();
  configure(&defaults);
  partNo = readStatus(TI_CCxxx0_PARTNUM);
  version = readStatus(TI_CCxxx0_VERSION);
  
  FsmState = State::Init;
}
//------------------------------------------------------------------------------
void A1101R09x::FSM()
{
  marcState = MarcState::Validate(readStatus(TI_CCxxx0_MARCSTATE));

  switch(marcState)
  {
  case MarcState::RXFIFO_OVERFLOW:
  case MarcState::TXFIFO_UNDERFLOW:
    //FsmState = State::Exception;
    break;
  }
  
  led.FSM();
  
  switch(FsmState)
  {
  case State::Init:
    readStatus(TI_CCxxx0_LQI);
    readStatus(TI_CCxxx0_RXBYTES & TI_CCxxx0_NUM_RXBYTES);
    writeStrobe(TI_CCxxx0_SRX);
    FsmState = State::Idle;
    break;
  case State::Idle:
    break;
  case State::Receiving:
    if(packetReady)
    {
      packetReady = false;
      pb.UnpackCC1101();
      if(unpack())
            FsmState = State::RxEnded;
      else
      {
        pb.Clear();
        FsmState = State::Idle;
      }
    }
    break;
  case State::RxEnded:
    cb->Args[0] = (intptr_t)&Buffer;
    cb->Process(Process::Parse_Radio_Message);
    Buffer.Clear();
    writeStrobe(TI_CCxxx0_SRX);
    FsmState = State::Idle;
    break;
  case State::Transmitting:
    tx();
    break;
  case State::TxEnded:
    FsmState = State::Idle;
    writeStrobe(TI_CCxxx0_SRX);
    break;
  case State::Exception:
    led.SetPattern(LedPattern::Toggle_100ms);
    break;
  }
}
//------------------------------------------------------------------------------
void A1101R09x::ISR()
{
  switch(FsmState)
  {
  case State::Init:
    break;
  case State::Idle:
  case State::Receiving:
    if(packetReady = receivePacket())
      FsmState = State::Receiving;
    else
      FsmState = State::Idle;
    break;
  case State::Transmitting:
    //Switch to Rx to receive acknowledge
    break;
  case State::TxWait:
    if(txPacketNo > txPackets)
      FsmState = State::TxEnded;
    else
      FsmState = State::Transmitting;
    break;
  }
}
//------------------------------------------------------------------------------
bool A1101R09x::SendPacket(uint8_t* msg, uint16_t len)
{
  if(FsmState == State::Idle)
  {
    Buffer.AddArray(msg, len);
    calcTxPackets(len);
    tx();
    return true;
  }
  return false;
}
//------------------------------------------------------------------------------
bool A1101R09x::unpack()
{
  bool ans = false;
  txPacketNo = pb.Get<uint8_t>(0);
  txPackets = pb.Get<uint8_t>(1);
  
  if(txPacketNo == 0)
    Buffer.Clear();
  
  Buffer.AddArray(pb.GetPointer(HEADER_BYTES),pb.Length()-HEADER_BYTES);
  pb.Clear();
  
  if(txPacketNo == txPackets)
    ans = true;

  return ans;
}
//------------------------------------------------------------------------------
void A1101R09x::calcTxPackets(uint16_t len)
{
  txPackets = len / (MAX_PAYLOAD_SIZE - HEADER_BYTES);
  //txPackets += (len & (MAX_PAYLOAD_SIZE - HEADER_BYTES)) ? 1 : 0;
  txPacketNo = 0;
}
//------------------------------------------------------------------------------
void A1101R09x::SendPacket(BufferHandler* b)
{
  /*b->PackCC1101();
  writeBurst(TI_CCxxx0_TXFIFO,b->GetPointer(0),b->Length());
  Buffer.Clear();
  FsmState = State::Tx;
  writeStrobe(TI_CCxxx0_STX);*/
}
//------------------------------------------------------------------------------
void A1101R09x::tx()
{
  led.SetPattern(LedPattern::On);
    
  uint8_t count = 0, ch = 0;
  
  pb.Add<uint8_t>(txPacketNo++);
  pb.Add<uint8_t>(txPackets);
  
  while(count < MAX_PAYLOAD_SIZE)
  {
    if(Buffer.TakeInt8(&ch))
    {
      pb.Add<uint8_t>(ch);
      count++;
    }
    else
    {
      Buffer.Clear();
      break;
    }
  }
  
  count = pb.Length();
  
  if(count > 0)
  {
    pb.PackCC1101();
    writeBurst(TI_CCxxx0_TXFIFO,pb.GetPointer(0),pb.Length());
    writeStrobe(TI_CCxxx0_STX);
    pb.Clear();
    FsmState = State::TxWait;
  }
  else
  {
    FsmState = State::Idle;
    writeStrobe(TI_CCxxx0_SRX);
  }
  
  led.SetPattern(LedPattern::Off);
}
//------------------------------------------------------------------------------
bool A1101R09x::receivePacket()
{
  led.SetPattern(LedPattern::On);
  
  uint16_t len = 0;
  uint8_t status = 0;
  bool ans = false;
   
  status = readStatus(TI_CCxxx0_LQI);
  
  if(status&0x80) //CRC_OK
  {
    len = readStatus(TI_CCxxx0_RXBYTES & TI_CCxxx0_NUM_RXBYTES);
    if(len) 
    {
      readBurst(TI_CCxxx0_RXFIFO,&pb, len);
      ans = true;
    }
  }
  else
  {
    writeStrobe(TI_CCxxx0_SFRX); //Flush RX FIFO
  }
  
  led.SetPattern(LedPattern::Off);
  
  return ans;
}
//------------------------------------------------------------------------------
void A1101R09x::reset()
{
  cs->Set(); HAL::Wait_us(30);
  cs->Clear(); HAL::Wait_us(30);
  cs->Set(); HAL::Wait_us(45);
  writeStrobe(TI_CCxxx0_SRES);
  HAL::Wait_us(100);
  writeStrobe(TI_CCxxx0_SFRX);
  writeStrobe(TI_CCxxx0_SFTX);
}
//------------------------------------------------------------------------------
bool A1101R09x::isChannelClear()
{
  uint8_t ans = readStatus(TI_CCxxx0_PKTSTATUS);
  return ans & 0x10;
}
//------------------------------------------------------------------------------
void A1101R09x::configure(CC1101Config* c)
{
  uint8_t* cfg = (uint8_t*)c;
  
  for(uint32_t i=0; i< 0x2f; i++)
    writeReg(i, cfg[i]);
  
  writeBurst(TI_CCxxx0_PATABLE, (uint8_t*)&PATABLE, 1);
}
//------------------------------------------------------------------------------
void A1101R09x::writeStrobe(uint8_t strobe)
{
  cs->Clear();               // CS enable 
  spi->WriteByte(strobe);       // Send strobe
  cs->Set();                   // /CS disable
}
//------------------------------------------------------------------------------
bool A1101R09x::writeReg(uint8_t reg, uint8_t val)
{
  uint8_t ans = 0;
  cs->Clear();          // CS enable
  spi->WriteByte(reg);  // Send address
  spi->WriteByte(val);  // Send data
  cs->Set();            // CS disable    
  
  cs->Clear();          // CS enable
  spi->WriteByte(reg | TI_CCxxx0_READ_SINGLE);  // Send address
  ans = spi->ReadByte();  // Read data
  cs->Set();            // CS disable   
  
  return ans == val;
}
//------------------------------------------------------------------------------
uint8_t A1101R09x::readReg(uint8_t reg)
{
  uint8_t ans = 0;
  cs->Clear();          // CS enable
  spi->WriteByte(reg | TI_CCxxx0_READ_SINGLE);  // Send address
  ans = spi->ReadByte();  // Read data
  cs->Set();            // CS disable    
  return ans;
}
//------------------------------------------------------------------------------
void A1101R09x::writeBurst(uint8_t addr, uint8_t* buff, uint16_t len)
{
  cs->Clear();                                  // CS enable
  spi->WriteByte(addr | TI_CCxxx0_WRITE_BURST); // Send address
  
  for (uint_fast16_t i = 0; i < len; i++)
    spi->WriteByte(buff[i]);                    // Send data
  
  cs->Set();                                    // CS disable
}
//------------------------------------------------------------------------------
void A1101R09x::readBurst(uint8_t addr, BufferHandler* b, uint16_t len)
{
  cs->Clear();                                  // CS enable
  spi->WriteByte(addr | TI_CCxxx0_READ_BURST);  // Send address

  for(uint_fast16_t i = 0; i < len; i++)
    b->Add<uint8_t>(spi->ReadByte());                  // Get data
  
  cs->Set();                                    // CS disable
}
//------------------------------------------------------------------------------
uint8_t A1101R09x::readStatus(uint8_t addr)
{
  uint8_t status = 0;
  cs->Clear();                                  // CS enable
  spi->WriteByte(addr | TI_CCxxx0_READ_BURST);  // Send address
  status = spi->ReadByte();                     // Get data
  cs->Set();                                    // CS disable

  return status;
}
//------------------------------------------------------------------------------

