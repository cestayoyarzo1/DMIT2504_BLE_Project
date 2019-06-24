/*
 * comhandler.h
 *
 *  Created on: Sep 16, 2016
 *      Author: Tom Diaz
 */

#ifndef COMHANDLER_H_
#define COMHANDLER_H_

#include <x_handler.h>

class COMHandler : public x_Handler
{
public:
  COMHandler(){};
  ~COMHandler(){};
  void Add(COMPort* p);
  COMPort* Get(uint32_t p);
protected:
private:
  void fsm();
  COMPort* port[NUMBER_OF_COM_PORTS];
  uint8_t activePorts;
};

#endif /* COMHANDLER_H_ */
