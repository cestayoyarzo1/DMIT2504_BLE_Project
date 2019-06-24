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
    fclk_2,
    fclk_4,
    fclk_8,
    fclk_16,
    fclk_32,
    fclk_64,
    fclk_128,
    fclk_256,
  };
};

#endif /* GLOBAL_CONSTANTS_H_ */
