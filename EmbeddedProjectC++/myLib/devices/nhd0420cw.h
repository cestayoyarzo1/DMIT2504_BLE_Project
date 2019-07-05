/*
  FILE: nhd0420cw.h
  Created on: 9/17/2016, by Tom Diaz
*/

#ifndef NHD_0420CW_H
#define NHD_0420CW_H

class NHD_0420CW : public SPIDevice //New Haven OLED Display
{
public:
  void testMethod();
  bool isBusy();
  void Clear();
  void Clear(uint8_t row);
  void turn(bool state);
  void WriteString(uint8_t row, uint8_t col, uint8_t* msg);
  void WriteChar(uint8_t row, uint8_t col, uint8_t c);
  void WriteLine(uint8_t row,uint8_t* msg);
  void WriteCentered(uint8_t row, const char* msg);
  void ShowScreen(void* screen);
  void SetContrast(uint8_t contrast);
  void BufferToScreen(uint8_t row, uint8_t col);
  BufferHandler Buffer;
private:
  void getID();
  void extraInit();
  void command(uint8_t cmd);
  void writeData(uint8_t data);  
  void writeRAM(uint8_t* str);
  void writeRAM(uint8_t c);
  void go(uint8_t row, uint8_t col);
  uint8_t buffer[32];
  //uint8_t dBuffer[80];
};

#endif /* NHD_0420CW_H */
