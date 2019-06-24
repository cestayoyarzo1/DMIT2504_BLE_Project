#include <hal.h>

void SensorPIN::Set()
{
  port->BSRR |= gpioSet;
}
//------------------------------------------------------------------------------
void SensorPIN::Reset()
{
  port->BSRR |= gpioReset;
}
//------------------------------------------------------------------------------
void D1820::Init(GPIO_TypeDef* port, uint16_t pin, TIM_TypeDef* timer)
{
  sensorPin.port = port;
  sensorPin.pin = pin;
  sensorPin.moderMask = 3 << pin*2; 
  sensorPin.moderGpo = 1 << pin*2; 
  sensorPin.gpioSet = 1 << pin;
  sensorPin.gpioReset = 1 << pin+16;
  sensorPin.moderAlt = 2 << pin*2;
  sensorPin.idr = 1<<pin;
  sensorPin.highSpeed = 3 << pin*2;  
  if(pin>7)
    sensorPin.port->AFR[1] |= 14<<(pin-8)*4;
  else
    sensorPin.port->AFR[0] |= 14<<pin*4;                          
  counter = timer;
  capState = open;
  capFalling = 0;
  capRaising = 0;
  inputPulse =0;
  fsmState = Idle;
  prevState = Idle;
  newReading =false;
}
//------------------------------------------------------------------------------
void D1820::CountUs(uint16_t cTime)
{
    timeUp =false;
    counter->ARR = cTime;                                      //sets pulse time
    counter->CR1 |= TIM_CR1_CEN;                               //Starts  counter
}
//------------------------------------------------------------------------------
void D1820::DelayUs(uint16_t dTime)
{
    timeUp =false;
    counter->ARR = dTime;                                      //sets pulse time
    counter->CR1 |= TIM_CR1_CEN;                               //Starts  counter
    while(!timeUp);                                            //wait until counter finishes  
}
//------------------------------------------------------------------------------
void D1820::TxMode()
{
  sensorPin.port->MODER &= ~sensorPin.moderMask;               //Clear moder
  sensorPin.port->MODER |= sensorPin.moderGpo;                 //General purpose output
  //sensorPin.port->OSPEEDR |= sensorPin.highSpeed;            //Set to very high speed GPIO
}
//------------------------------------------------------------------------------
void D1820::RxMode()    //this function releases the BUS and uses timer to capture pulses
{
    sensorPin.port->MODER &= ~sensorPin.moderMask;               //Clear moder
    sensorPin.port->MODER |= sensorPin.moderAlt;                //Alt function                                   //Start timer
}
//------------------------------------------------------------------------------
void D1820::InputMode()//This function releases the BUS and uses uses GPIO to capture pulses
                      //Still needs to add soft. counter and GPIO interrupts
{
  sensorPin.port->MODER &=~sensorPin.moderMask;
}

//------------------------------------------------------------------------------
void D1820::SinglePulse(uint16_t time ) //minimum 3uS
{
    TxMode();
    sensorPin.Reset();
    if (time>2)
      DelayUs(time-2);                          //This is to adjust time lost
    sensorPin.Set();
}
//------------------------------------------------------------------------------
void D1820::StartCapture()
{
  fsmState = Start;
}
//------------------------------------------------------------------------------
void D1820::StopCapture()
{
  fsmState = Idle;
}
//------------------------------------------------------------------------------
void D1820::InitializePulse()
{//use this pulse at the beginning of any communication with the sensor
  SinglePulse(480);
  //InputMode();
  RxMode();
}
//------------------------------------------------------------------------------
void D1820::FallingEdde(uint16_t capture)
{
  pulledLow = true;
  capFalling = capture;
  capState = Low;
}
//------------------------------------------------------------------------------
void D1820::RisingEdge(uint16_t capture)
{ 

  if(capState ==Low)
  {
    capRaising = capture;
    inputPulse = capRaising - capFalling;
    fsmState = Response;   
  } 
  capState = High;
  pulledLow = false; 
}
//------------------------------------------------------------------------------
void D1820::FSM()
{
  switch(fsmState)
  {
    case(Idle):
      break;
    
    case Start:
      InitializePulse();
      prevState = Start;
      fsmState = Wait;
      break; 
      
    case Wait:          //here waits for the response pulse to send next command
      break;
      
    case Response:
      if((inputPulse > 60) && (inputPulse <240))
      {
        switch(prevState)  //Initialization pulse not necesary here since it was done on the prev state
        {
        case Start:
          TempConversion();
          fsmState = Converting;
          msTimer.Start();
          break;
     
        case Converting:
          ReadScratchPad(); //Scratchpad Array is populated here
          fsmState = Calculate;
          break;
        } 
      }
      break;
        
    case Converting:
      if(msTimer.Compare(750) && ReadTimeSlot())
      {
        InitializePulse();
        msTimer.StopAndReset();
        prevState = Converting; 
        fsmState = Wait;
      }
      break;
      
    case Calculate:
      if (CalculateTemp())
        newReading =true;
      fsmState= Idle;
      break;      
  }
}
//------------------------------------------------------------------------------
void D1820::WriteBit(bool bit) ///write time slot 64[us] total
{
  switch(bit)
  {
  case true:
    TxMode();
    SinglePulse(8);
    //InputMode();
    RxMode();
    DelayUs(56);
    break;
  case false:
    TxMode();
    SinglePulse(62);
    RxMode();
    //InputMode();
    DelayUs(2);
    break; 
  }
}
//------------------------------------------------------------------------------
void D1820::WriteHex(uint8_t data)
{
  uint8_t index;
  bool bitOut;
  
  for(index=0;index<8;index++)
  {
    if(data & (1<<index))
      bitOut = true;
    else
      bitOut = false;
    WriteBit(bitOut);
  }
}
//------------------------------------------------------------------------------
uint8_t D1820::ReadTimeSlot()
{ 
    uint8_t bitIn;
    SinglePulse(3);
    //InputMode();                  //release the bus
    RxMode();                      //release the bus
    DelayUs(6);                   //wait to receive bit
    if(sensorPin.port->IDR & sensorPin.idr)
    {
      bitIn =1;
      pulledLow = false;
    }
    else
      bitIn = 0;
    DelayUs(50);                 //delay to make read time slot 64[us] total
    return bitIn;
}
//------------------------------------------------------------------------------
void D1820::ReadScratchPad()
{
    ClearScratchPadArray();
    WriteHex(0xCC);                          //simply SKIP ROM command
    WriteHex(0xBE);            	            // read scratchpad command
    uint8_t i=0;
    uint8_t j=0;
    uint8_t BIT=2;    
    for (j=0;j<9;j++)        				// repeat 9 times read byte from 0 to 8
        for (i=0;i<8;i++)   				 //read byte
        {
            BIT= ReadTimeSlot();
            if(BIT == 1)
                scratchPad[j] |= 1<<i;
            else
            if(BIT == 0)
                scratchPad[j] &= ~(1<<i);
        }   
    DelayUs(200);
}
//------------------------------------------------------------------------------
bool D1820::CalculateTemp()
{
  //int nTemp = 0;
  //int16_t tmp = 0;
  float decimal = 0.0;
  
  nTemp = scratchPad[1];        //get temp MSB
  nTemp <<= 8;                  //shifts to the upper end;
  nTemp |= scratchPad[0];       //adds LSB
  if (nTemp<0)
    tmp = nTemp >> 4 | 0xF000;
  else
    tmp = nTemp >> 4;
  decimal = 0.5 * (nTemp & 0x8) + 0.25 * (nTemp & 0x4) + 0.125 * (nTemp & 0x2) + 0.0625 * (nTemp & 0x1);
  if (Check_CRC())
  {
    temperature = tmp + decimal;
    return true;
  }
  else
    return false;

}
//------------------------------------------------------------------------------
void D1820::TempConversion()
{
  WriteHex(0xCC);               //simply SKIP ROM command
  WriteHex(0x44);               //convert command
}
//------------------------------------------------------------------------------
void D1820::ClearScratchPadArray()
{  
  memset(scratchPad,0,9);
}
//------------------------------------------------------------------------------
bool D1820::Check_CRC()
{
  uint8_t bit, temp_mask, shiftReg,i,j,xor_bit;
  
  shiftReg = 0;
  for(i = 0; i < 9; i++)
    for(j = 0;j < 8; j++)
    {
        bit = (scratchPad[i] & (0x01<<j))>>j;                   //extract bit "j" from byte "i" 
        xor_bit = (shiftReg & 0x01) ^ bit;                      //xor with lsb from shiftreg
        temp_mask = xor_bit << 4 | xor_bit << 3;		//creates mask byte with "xored" bit
        shiftReg = (shiftReg ^ temp_mask) >> 1 | xor_bit << 7;	//performs exclusive or and shift to the right adding lsb xor on the msb
    }
  if(shiftReg ==0)
    return true;
      else
        return false;
}
//-----------------------------------------------------------------------------
float D1820::GetTemperature()
{
  return temperature;
}
//------------------------------------------------------------------------------
bool D1820::NewReading()
{
  if(newReading)
  {
    newReading = false;
    return true;
  }
  else
    return false;  
}
