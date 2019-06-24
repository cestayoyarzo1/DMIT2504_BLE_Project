#ifndef GLOBAL_CONSTANTS_H_
#define GLOBAL_CONSTANTS_H_

#define EVER 1
#define CALLBACK_BUFFER_SIZE 32
#define UID_LEN 24

class SpiClk
{
public:
  enum Value
  {
    f_40M,
    f_20M,
    f_10M,
    f_5M,
    f_2M5,
    f_1M25,
    f_625K,
    f_312K5,
  };
};

#endif /* GLOBAL_CONSTANTS_H_ */
