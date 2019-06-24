/*
 * x_system.h
 *
 *  Created on: Sep 16, 2016
 *      Author: Tomas Diaz
 */

#ifndef X_SYSTEM_H_
#define X_SYSTEM_H_

#include "stdint.h"
#include "stopwatch.h"
#include "global_constants.h"
#include "x_callback.h"
   

class x_System : public x_Callback
{
public:
  x_System();
  ~x_System(){}
private:
protected:
  Stopwatch paceTimer;
  uint8_t buffer[CALLBACK_BUFFER_SIZE];
};

#endif /* X_SYSTEM_H_ */
