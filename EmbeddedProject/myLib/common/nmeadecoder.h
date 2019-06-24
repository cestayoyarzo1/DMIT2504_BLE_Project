/*
  FILE: nmeadecoder.h
  Created on: 9/27/2017, by Tom Diaz
*/
#ifndef NMEADECODER_H
#define NMEADECODER_H

#include "stdint.h"
#include "buffer.h"

#define MAX_NMEA_STING_LENGTH 82

class NMEADecoder
{
public:
  NMEADecoder()
  {
    lat = 29.763878f; //Default location
    lon = -95.385814f;
  };
  ~NMEADecoder(){};
  bool Decode(BufferHandler* b);
  uint8_t* GetBuffer();
  bool Lock() { return lock; }
  double Latitude() { return lat; }
  double Longitude() { return lon; }
protected:
  bool lock;
  double lat;
  double lon;
  bool GNGGA();
  bool getCoordenate(uint8_t* b, uint8_t sign, double* coord);
  uint8_t buffer[MAX_NMEA_STING_LENGTH+1];
private:
};

#endif /* NMEADECODER_H */
