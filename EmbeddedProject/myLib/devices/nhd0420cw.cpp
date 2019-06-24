/*
  FILE: nhd0420cw.cpp
  Created on: 9/17/2016, by Tom Diaz
*/

#include <hal.h>
#include <screens.h>
#include <string.h>
#include <nhd0420cw.h>
/*
------------------------------------NOTES---------------------------------------
Newhaven's NHD 0420CW datasheet states the use of a start byte before any
read or read transaction. 0xF8 for write and 0xF9 for read. Start byte has to be
sent Most Signicant Bit first according do datasheet diagrams.
After sending the start byte, next bytes (parameters, data) have to be split in
two bytes per byte ie; 0xEA becomes 0xA0 + 0xE0. BUT these bytes have to be sent
Least Significant Bit first WTF!!!
An easy but ineffcient way to overcome this inconsistency is to create a 
function to reverse bit order of a byte and return a new byte.
A more difficult but seamless way to fix this is to configure SPI peripheral to
send frames as LSB first and change start bytes from 0xF8 to 0x1F and 0xFA to
0x5F (reversed). Data/Parameter 0xEA becomes 0x0A + 0x0E.
*/

void NHD_0420CW::getID()
{
  
}
//------------------------------------------------------------------------------
void NHD_0420CW::extraInit()
{
  command(0x2A); //function set (extended command set)
  command(0x71); //function selection A
  writeData(0x00); // disable internal VDD regulator (2.8V I/O). data(0x5C) = enable regulator (5V I/O)
  command(0x28); //function set (fundamental command set)
  command(0x08); //display off, cursor off, blink off
  command(0x2A); //function set (extended command set)
  command(0x79); //OLED command set enabled
  command(0xD5); //set display clock divide ratio/oscillator frequency
  command(0x70); //set display clock divide ratio/oscillator frequency
  command(0x78); //OLED command set disabled
  command(0x09); //extended function set (4-lines)
  command(0x06); //COM SEG direction
  command(0x72); //function selection B
  writeData(0x00); //ROM CGRAM selection
  command(0x2A); //function set (extended command set)
  command(0x79); //OLED command set enabled
  command(0xDA); //set SEG pins hardware configuration
  command(0x10); //set SEG pins hardware configuration
  command(0xDC); //function selection C
  command(0x00); //function selection C
  command(0x81); //set contrast control
  command(0x7F); //set contrast control
  command(0xD9); //set phase length
  command(0xF1); //set phase length
  command(0xDB); //set VCOMH deselect level
  command(0x40); //set VCOMH deselect level
  command(0x78); //OLED command set disabled
  command(0x28); //function set (fundamental command set)
  command(0x01); //clear display
  command(0x80); //set DDRAM address to 0x00
  command(0x0C); //display ON, invisible cursor
  //command(0x0F); //display ON, visible cursor
  Buffer.Init(buffer,32);
}
//------------------------------------------------------------------------------
void NHD_0420CW::BufferToScreen(uint8_t row, uint8_t col)
{
  WriteString(row,col,Buffer.GetPointer(0));
}
//------------------------------------------------------------------------------
void NHD_0420CW::SetContrast(uint8_t contrast)
{
  command(0x2A); //function set (extended command set)
  command(0x79); //OLED command set enabled
  command(0x81); //set contrast control
  command(contrast); //set contrast
  command(0x78); //OLED command set disabled
  command(0x28); //function set (fundamental command set)
}
//------------------------------------------------------------------------------
void NHD_0420CW::Clear()
{
  command(0x01); //clear display
  command(0x80); //set DDRAM address to 0x00
}
//------------------------------------------------------------------------------
void NHD_0420CW::Clear(uint8_t row)
{
  WriteLine(row,(uint8_t*)clearLine);
}
//------------------------------------------------------------------------------
void NHD_0420CW::testMethod()
{
  WriteString(2,2,(uint8_t*)"Me pica la panza\0");
}
//------------------------------------------------------------------------------
void NHD_0420CW::turn(bool state)
{
  state?command(0x0C):command(0x08);
}
//------------------------------------------------------------------------------
void NHD_0420CW::go(uint8_t row, uint8_t col)
{
  uint8_t cmd = 0x80;
  cmd |= (0x20 * row) + col;
  command(cmd);
}
//------------------------------------------------------------------------------
void NHD_0420CW::WriteString(uint8_t row, uint8_t col, uint8_t* msg)
{
  go(row,col);
  writeRAM(msg);
}
//------------------------------------------------------------------------------
void NHD_0420CW::WriteLine(uint8_t row, uint8_t* msg)
{
  go(row,0);
  writeRAM(msg);
}
//------------------------------------------------------------------------------
void NHD_0420CW::WriteCentered(uint8_t row, const char* msg)
{
  int16_t len = strlen(msg);
  uint8_t col = 20-len;
  Clear(row);
  if(col>0)
  {
    col/=2;
    WriteString(row,col,(uint8_t*)msg);
  }
  else
    WriteLine(0,(uint8_t*)msg);
  //WriteLine(col>0?col/2:0,(uint8_t*)msg);
}
//------------------------------------------------------------------------------
void NHD_0420CW::WriteChar(uint8_t row, uint8_t col, uint8_t c)
{
  go(row,col);
  writeRAM(c);
}
//------------------------------------------------------------------------------
void NHD_0420CW::ShowScreen(void* screen)
{
  WriteLine(0,(uint8_t*)screen);
  WriteLine(1,(uint8_t*)screen+21);
  WriteLine(2,(uint8_t*)screen+42);
  WriteLine(3,(uint8_t*)screen+63);
}
//------------------------------------------------------------------------------
void NHD_0420CW::command(uint8_t cmd)
{  
  cs->Clear();
  spi->WriteByte(0x1F);
  spi->WriteByte(cmd&0x0f);
  spi->WriteByte(cmd>>4);
  cs->Set();
  while(isBusy());
}
//------------------------------------------------------------------------------
bool NHD_0420CW::isBusy()
{
  uint8_t data=0;
  cs->Clear();
  spi->WriteByte(0x3F);
  data = spi->ReadByte();
  cs->Set();
  return data & 0x80;
  
  /*gs.Delay(5);
  return false;*/
}
//------------------------------------------------------------------------------
void NHD_0420CW::writeData(uint8_t data)
{
  cs->Clear();
  spi->WriteByte(0x5F);
  spi->WriteByte(data&0x0f);
  spi->WriteByte(data>>4);
  cs->Set();
  while(isBusy());
}
//------------------------------------------------------------------------------
//Writes data to RAM (burst)
void NHD_0420CW::writeRAM(uint8_t* str)
{
  uint8_t i=0;
  cs->Clear();
  spi->WriteByte(0x5F);
  while(str[i])
  {
    spi->WriteByte(str[i]&0x0f);
    spi->WriteByte(str[i]>>4);
    i++;
  }
  cs->Set();
  while(isBusy());
}
//------------------------------------------------------------------------------
//Writes data to RAM (burst)
void NHD_0420CW::writeRAM(uint8_t c)
{
  cs->Clear();
  spi->WriteByte(0x5F);
  spi->WriteByte(c&0x0f);
  spi->WriteByte(c>>4);
  cs->Set();
  while(isBusy());
}