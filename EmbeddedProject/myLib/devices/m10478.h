/*
  FILE: m10478.h
  Created on: 10/17/2017, by Tom Diaz
*/
#ifndef M10478_H
#define M10478_H

#include <comport.h>
#include <x_gpio.h>
#include <stopwatch.h>
#include <nmeadecoder.h>

class M10478
{
public:
  enum class State
  {
    Init,
    WaitLowInit,
    SetHighInit,
    NormalOp,
  };
  M10478()
  {
    lat = 29.763878f; //Default location Houston Tx
    lon = -95.385814f;
  };
  ~M10478(){};
  void Init(x_GPIO* en_);
  void FSM();
  void Parse(COMPort* p);
  float Longitude() { return lon; }
  float Latitude() { return lat; }
  NMEADecoder* Decoder() { return &decoder; }
protected:
  float lon;
  float lat;
  NMEADecoder decoder;
  x_GPIO* en;
  State state;
  uint32_t timer;
private:
};


#endif /* M10478_H */
