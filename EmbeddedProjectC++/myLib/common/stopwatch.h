/*
 * stopwatch.h
 *
 *  Created on: Jun 10, 2016
 *      Author: Tomas Diaz
 */

#ifndef STOPWATCH_H_
#define STOPWATCH_H_

#include <stdint.h>

class Stopwatch
{
  public:
    Stopwatch();
    uint32_t Read();
    void Restart();
    void Reset();
    void Stop();
    void Start();
    void Delay(uint16_t delay);
    bool Compare(uint16_t period);
    void StopAndReset();
    bool isRunning();
  private:
    volatile uint32_t start;
    volatile uint32_t count;
    volatile bool running;
};

#endif /* STOPWATCH_H_ */
