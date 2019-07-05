/*
  FILE: simplelist.h
  Created on: 10/6/2017, by Tom Diaz
*/
#ifndef SIMPLELIST_H
#define SIMPLELIST_H

#include <type_traits>
//#include <arm_math.h>

template <typename T>  class SimpleList
{
  static_assert(std::is_integral<T>::value, "Arythmetic type required");
public:
  SimpleList()
  {
    newData = false;
  };
  ~SimpleList(){};
  void Init(T* ptr_, uint16_t size_)
  {
    ptr = ptr_;
    size = size_;
    len = 0;
  }
  bool Add(T val)
  {
    if(len < size)
    {
      newData = true;
      ptr[len++] = val;
      return false;
    }
    return true;
  }
  T Get(uint16_t loc)
  {
    if((loc <  len) && (loc < size))
    {
      return ptr[loc];
    }
    return 0;
  }
  T GetLast()
  {
    if(len>0)
      return ptr[len-1];
    else
      return 0;
  }
  
  void Clear()
  {
    newData = false;
    len = 0;
  }
  uint16_t Length()
  {
    return len;
  }
  bool NewData()
  {
    if(newData)
    {
      newData = false;
      return true;
    }
    return false;
  }
  T* Data()
  {
    return ptr;
  }
  
  T Deviation()
  {
    T min = 0, max = 0;
    for(uint32_t i = 0; i < len; i++)
    {
      if(ptr[i] > max)
        max = ptr[i];
    }
    min = max;
    for(uint32_t i = 0; i < len; i++)
    {
      if(ptr[i] < min)
        min = ptr[i];
    }
    return max - min;
  }
  
  T Average()
  {
    int64_t result = 0;
    for(uint32_t i = 0; i < len; i++)
      result += (int64_t)ptr[i];
    result /= len;
    return (T)result;
  }
protected:
  T* ptr;
  bool newData;
  uint16_t size;
  uint16_t len;
private:
};


#endif /* SIMPLELIST_H */
