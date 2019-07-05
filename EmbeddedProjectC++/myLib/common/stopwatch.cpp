/*
 * stopwatch.cpp
 *
 *  Created on: Jun 10, 2016
 *      Author: Tomas Diaz
 */

/*
  Relies on HAL::OneMilliSecondSnapshot(). Platform dependent.
  Stopwatch class can create near infinite number of non dynamic stopwatches to
  use as timebase.
*/



#include <hal.h>	// Platform dependent
#include <stopwatch.h>

//--------------------------------------------------------Stopwatch::Constructor
Stopwatch::Stopwatch()
{
  running=false;
  start=0;
  count=0;
}
//------------------------------------------------------------Stopwatch::Delay()
void Stopwatch::Delay(uint16_t delay)
{
  Start();
  while(Read()<=delay);
  StopAndReset();
}
//-------------------------------------------------------------Stopwatch::Read()
uint32_t Stopwatch::Read()
{
  if(running)
  {
    uint32_t snapshot = HAL::OneMilliSecondSnapshot();

    
    if(snapshot>=start)
      return(snapshot-start);
    else
      return((0xFFFFFFFF-start)+snapshot);
  }
  else
    return(count);
}
//----------------------------------------------------------Stopwatch::Restart()
void Stopwatch::Restart()
{
  start = HAL::OneMilliSecondSnapshot();
  running=true;
  count=0;
}
//------------------------------------------------------------Stopwatch::Reset()
void Stopwatch::Reset()
{
  start = HAL::OneMilliSecondSnapshot();
  count=0;
}
//------------------------------------------------------------Stopwatch::Reset()
bool Stopwatch::Compare(uint16_t period)
{
  if(Read()>=period)
  {
    Reset();
    return(true);
  }
  else
    return(false);
}
//-------------------------------------------------------------Stopwatch::Stop()
void Stopwatch::Stop()
{
  if(running)
  {
    count+=Read();
    running=false;
  }
}
//------------------------------------------------------------Stopwatch::Start()
void Stopwatch::Start()
{
  if(!running)
  {
    start = HAL::OneMilliSecondSnapshot();
    running=true;
  }
}
//-----------------------------------------------------Stopwatch::StopAndReset()
void Stopwatch::StopAndReset()
{
  if(running)
  {
    Stop();
    Reset();
  }
}
//------------------------------------------------------------------------------
bool Stopwatch::isRunning()
{
  return(running);
}
//------------------------------------------------------------------------------


