/*
 * d1820.h
 *
 *  Created on: Sep 25th, 2017
 *      Author: Carlos Estay
 * 
 * Template to interact with the Maxim Integrated
 * DS18B20 1-wire temperature sensor
 */
#ifndef D1820_H
#define D1820_H

class SensorPIN
{
friend class D1820;
public:
  void Set();
  void Reset();
private:
  GPIO_TypeDef* port;
  uint16_t pin;
  uint32_t moderMask;
  uint32_t moderGpo;
  uint32_t gpioSet;
  uint32_t gpioReset;
  uint32_t moderAlt;
  uint32_t idr;
  uint32_t afrTimer;  
  uint32_t highSpeed;
};   
//------------------------------------------------------------------------------
class D1820
{
public:
  void Init(GPIO_TypeDef* port, uint16_t pin, TIM_TypeDef* timer);
  void CountUs(uint16_t rTime);
  void DelayUs(uint16_t dTime);
  void TxMode();
  void RxMode();
  void InputMode();
  void SinglePulse(uint16_t time);
  void StartCapture();
  void StopCapture();
  void InitializePulse();
  void WriteBit(bool bit);
  void WriteHex(uint8_t data);
  uint8_t ReadTimeSlot();
  void FallingEdde(uint16_t capture); 
  void RisingEdge(uint16_t capture);
  void ReadScratchPad();
  void CrcCheck(uint8_t* byte);
  bool CalculateTemp();
  void TempConversion(); 
  void FSM();
  void ClearScratchPadArray();
  float GetTemperature();
  bool Check_CRC();
  bool timeUp;
  bool NewReading();
  
private:
  bool pulledLow;
  TIM_TypeDef * counter;
  uint16_t capFalling;
  uint16_t capRaising;
  uint16_t inputPulse;
  int16_t nTemp;
  int16_t tmp;
  Stopwatch msTimer;
  uint8_t convSuccess;
  float temperature;
  uint8_t scratchPad[9];
  bool newReading;

  enum FsmState
  {
    Idle,
    Start,
    Wait,
    Response,
    Converting,
    ReadScr,
    Calculate,
  };
  
  enum InPulseState
  {
    Low,
    High,
    open,
  };
  SensorPIN sensorPin;
  FsmState fsmState;
  FsmState prevState;
  InPulseState capState;
};
#endif /* D1820_H */