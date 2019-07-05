/*
 * x_handler.h
 *
 *  Created on: Sep 16, 2016
 *      Author: E1216431
 */

#ifndef X_HANDLER_H_
#define X_HANDLER_H_

#include <stdint.h>
//#include <x_system.h>
#include <x_callback.h>

class x_Handler
{
public:
  x_Handler(){};
  ~x_Handler(){};
  void Init(x_Callback* sys);
  void Init(x_Callback* sys, bool enable);
  void FSM(uint32_t interval);
  void Enable(bool val);
protected:
  virtual void fsm() = 0;
  virtual void childInit();
  bool enabled;
  uint32_t count;
  x_Callback* callback;
};


#endif /* X_HANDLER_H_ */
