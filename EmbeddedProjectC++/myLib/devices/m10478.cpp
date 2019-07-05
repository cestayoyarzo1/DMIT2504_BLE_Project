/*
  FILE: m10478.cpp
  Created on: 10/17/2017, by Tom Diaz
*/

#include <hal.h>
#include <m10478.h>

void M10478::Init(x_GPIO* en_)
{
  en = en_;
}

void M10478::FSM()
{
  switch(state)
  {
  case State::Init:
    en->Clear();
    timer = 0;
    state = State::WaitLowInit;
    break;
  case State::WaitLowInit:
    if(++timer >= 5000)
    {
      en->Set();
      timer = 0;
      state = State::NormalOp;
    }
    break;
  default:
    break;
  }
}

void M10478::Parse(COMPort* p)
{
  while(decoder.Decode(p->RxB()));
  lat = decoder.Latitude();
  lon = decoder.Longitude();
}